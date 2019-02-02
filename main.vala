errordomain Foodsoft {
    Error
}

class Job {
    Manager manager;
    public uint32 id;
    public int job_id = -1;
    public int job_state = -1;
    public string message = "";
    public bool pending = false;
    bool finished = false;
    uint timeout = 0;

    public Job(Manager manager, uint32 id) {
        this.manager = manager;
        this.id = id;
    }

    public async int run() throws Error {
        send_job_state();

        var message = manager.new_soup_message(@"/printer/$id", false);

        message.starting.connect((t) => {
            download_update("starting");
        });

        message.wrote_body.connect((t) => {
            download_update("sent request");
        });

        var stream = yield manager.send_http_request(message);

        var chunks = new SList<Bytes>();
        uint length = 0;

        for (;;) {
            var chunk = yield stream.read_bytes_async(8192);
            if (chunk == null || chunk.length < 1)
                break;
            chunks.append(chunk);
            length += chunk.length;
            download_update(@"received $length bytes");
        }

        download_update("finished");

        job_id = manager.create_cups_job(id, chunks);
        return job_id;
    }

    public void cancel() {
        if (finished)
            return;
        manager.cancel_job(job_id);
        finished = true;
    }

    public void download_update(string? message) {
        this.message = message;

        if (timeout != 0)
            return;

        timeout = Timeout.add(2500, () => {
            send_job_state();
            return false;
        });
    }

    public void cups_update(CUPS.IPP.JobState state, time_t completed_time) {
        if (job_state == state)
            return;
        if (completed_time != 0)
            finished = true;
        message = @"JOB #$job_id";
        job_state = state;
        send_job_state();
    }

    void send_job_state() {
        if (timeout != 0) {
            Source.remove(timeout);
            timeout = 0;
        }

        manager.send_job_state(this);
    }

    public char display_icon() {
        if (job_state == -1)
            return 'D';
        if (job_state == CUPS.IPP.JobState.PENDING)
            return 'E';
        if (job_state == CUPS.IPP.JobState.HELD)
            return 'H';
        if (job_state == CUPS.IPP.JobState.PROCESSING)
            return 'P';
        if (job_state == CUPS.IPP.JobState.STOPPED)
            return 'S';
        if (job_state == CUPS.IPP.JobState.CANCELED)
            return 'N';
        if (job_state == CUPS.IPP.JobState.ABORTED)
            return 'A';
        if (job_state == CUPS.IPP.JobState.COMPLETED)
            return 'C';
        return '\0';
    }

    public string websocket_text() {
        string text = @"{\"id\":$id";
        if (job_state == -1) {
            text += ",\"state\":\"downloading\"";
        } else {
            var state = "unknown";
            if (job_state == CUPS.IPP.JobState.PENDING)
                state = "pending";
            else if (job_state == CUPS.IPP.JobState.HELD)
                state = "held";
            else if (job_state == CUPS.IPP.JobState.PROCESSING)
                state = "processing";
            else if (job_state == CUPS.IPP.JobState.STOPPED)
                state = "stopped";
            else if (job_state == CUPS.IPP.JobState.CANCELED)
                state = "canceled";
            else if (job_state == CUPS.IPP.JobState.ABORTED)
                state = "aborted";
            else if (job_state == CUPS.IPP.JobState.COMPLETED)
                state = "completed";
            text += @",\"state\":\"$state\"";
        }
        if (message != "")
            text += @",\"message\":\"$message\"";
        if (finished)
            text += ",\"finish\":true";
        text += "}";
        print (@"==> $text\n");
        return text;
    }
}

class Manager {
    Soup.Session session = new Soup.Session ();
    Soup.WebsocketConnection connection;
    unowned CUPS.HTTP.HTTP http = CUPS.HTTP.DEFAULT;
    unowned CUPS.Destination[] destinations;
    unowned CUPS.Destination? destination;
    unowned CUPS.DestinationInfo* info;
    HashTable<uint32, Job> jobs = new HashTable<uint32, Job>(direct_hash, direct_equal);
    HashTable<int, Job> cups_jobs = new HashTable<int, Job>(direct_hash, direct_equal);
    SList<Job> pending = new SList<Job>();
    string url;
    string token;

    public delegate void DelegateCUPSJob(CUPS.Job* job);
    public delegate void DelegateJob(Job job);
    public signal void connection_state_changed(bool connected);

    public Manager(string url, string token, string? printerName = null) throws Foodsoft.Error {
        this.url = url;
        this.token = token;

        int destinations_length = http.get_dests (out destinations);
        destinations.length = destinations_length;

        destination = CUPS.Destination.get_dest (printerName, null, destinations);
        if (destination == null)
            throw new Foodsoft.Error("No destination found");

        info = http.copy_dest_info(destination);
    }

    ~Manager() {
        CUPS.Destination.free(destinations);
    }

    public async void run(Cancellable cancellable) {
        while (!cancellable.is_cancelled()) {
            if (yield connectWebSocket(cancellable))
                continue;

            GLib.Timeout.add(5000, () => {
                run.callback();
                return false;
            });
            yield;
        }
    }

    public void foreach_cups_job(DelegateCUPSJob cb) {
        unowned CUPS.Job[] jobs;
        int jobs_length = http.get_jobs(out jobs, destination.name, 1, CUPS.WhichJobs.ALL);
        jobs.length = jobs_length;

        for (int i = 0; i < jobs_length; ++i)
            cb(&jobs[i]);

        CUPS.Job.free(jobs);
    }

    public void foreach_job(DelegateJob cb) {
        foreach (var job in jobs.get_values())
            cb(job);
    }

    public int create_cups_job(uint32 id, SList<Bytes> chunks) throws Foodsoft.Error {
        string docname = @"$id";
        int job_id = http.create_job(destination.name, docname, null);
        if (job_id == 0)
            throw new Foodsoft.Error(CUPS.last_error_string());

        if (http.start_document(destination.name, job_id, docname, CUPS.Format.PDF, 1) != CUPS.HTTP.Status.CONTINUE) {
            http.cancel_dest_job(destination, job_id);
            throw new Foodsoft.Error(CUPS.last_error_string());
        }

        foreach (var chunk in chunks) {
            if (http.write_request_data(chunk.get_data()) != CUPS.HTTP.Status.CONTINUE) {
                http.cancel_dest_job(destination, job_id);
                throw new Foodsoft.Error(CUPS.last_error_string());
            }
        }

        http.finish_document(destination.name);

        return job_id;
    }

    public void cancel_job(int id) {
        http.cancel_dest_job(destination, id);
    }

    public void cancel_all_jobs() {
        foreach_cups_job ((cups_job) => {
            if (cups_job.completed_time == 0)
                cancel_job(cups_job.id);
        });
    }

    public void update_cups_jobs() {
        foreach_cups_job ((cups_job) => {
            unowned Job job = cups_jobs.get(cups_job.id);
            if (job != null)
                job.cups_update(cups_job.state, cups_job.completed_time);
        });
    }

    public Soup.Message new_soup_message(string path, bool xx = false) {
        var message = new Soup.Message("GET", @"$url$path");
        message.request_headers.append("Authorization", @"Bearer $token");
        return message;
    }

    public async InputStream send_http_request(Soup.Message message) throws Error {
        return yield session.send_async(message);
    }

    public void send_pending_job_states() {
        foreach (var job in pending)
            send_job_state(job);
        pending = new SList<Job>();
    }

    public void send_job_state(Job job) {
        if (connection != null) {
            connection.send_text(job.websocket_text());
            job.pending = false;
            return;
        }
        if (job.pending)
            return;
        pending.append(job);
        job.pending = true;
    }

    public void send_dummy() {
        connection.send_text("{}");
    }

    void set_unfinished_jobs(SList<uint32> list) {
        foreach (var id in list) {
            if (jobs.contains(id))
                continue;

            Job job = new Job(this, id);
            jobs[id] = job;

            job.run.begin((obj, res) => {
                try {
                    var job_id = job.run.end(res);
                    cups_jobs[job_id] = job;
                } catch (GLib.Error e) {
                    print("ERROR (Job): %s\n", e.message);
                }
            });
        }

        var finished = new SList<uint32>();
        jobs.for_each((id, job) => {
            if (list.index(id) == -1) {
                job.cancel();
                cups_jobs.remove(job.job_id);
                finished.append(id);
            }
        });

        foreach (var id in finished) {
            jobs.remove(id);
        }
    }

    async bool connectWebSocket(Cancellable cancellable) {
        try {
            var message = new_soup_message("/printer/socket");
            connection = yield session.websocket_connect_async(message, null, null, cancellable);
            connection_state_changed(true);

            send_pending_job_states();

            connection.message.connect((t, type, message) => {
                try {
                    var list = new SList<uint32> ();
                    var parser = new Json.Parser ();

                    parser.load_from_data((string) message.get_data(), message.length);

                    var root_object = parser.get_root ();
                    if (root_object.get_node_type() != Json.NodeType.OBJECT)
                      throw new Foodsoft.Error("Invalid JSON object");

                    var aa = root_object.get_object().get_member("unfinished_jobs");
                    if (aa == null || aa.get_node_type() != Json.NodeType.ARRAY)
                        throw new Foodsoft.Error("Invalid JSON array");

                    var unfinisehd_jobs = aa.get_array();
                    foreach (var item in unfinisehd_jobs.get_elements ()) {
                        list.append((uint32) item.get_int());
                    }

                    set_unfinished_jobs(list);
                } catch (GLib.Error e) {
                    print("ERROR (WebSocket): %s\n", e.message);
                }
            });

            connection.closed.connect((t) => {
                connectWebSocket.callback();
            });
            connection.error.connect((t, e) => {
                connectWebSocket.callback();
            });

            yield;

            connection_state_changed(false);
            connection = null;
        } catch (Error e) {
            print ("Websocket connection failed: %s\n", e.message);
            return false;
        }
        return true;
    }
}

class ModemChecker {
    DBusConnection connection;
    public int signal_level = -1;
    public bool connected = false;

    public ModemChecker() throws Error {
        connection = Bus.get_sync(BusType.SYSTEM);
    }

    public void update(Cancellable? cancellable) {
        try {
            var manager = new MM.Manager.sync(connection, DBusObjectManagerClientFlags.NONE, cancellable);
            var modems = manager.get_objects();
            if (modems != null) {
                var object = (MM.Object) modems.data;
                var modem = object.get_modem();
                bool recent;
                signal_level = 6 * 6 * (int) modem.get_signal_quality(out recent) / 100 / 5;
                connected = modem.get_state() >= MM.ModemState.CONNECTED;
            } else {
                signal_level = -1;
                connected = false;
            }
        } catch (GLib.Error e) {
            print("ERROR (MM): %s\n", e.message);
        }
    }
}

class UsbChecker {
    LibUSB.Context context;
    uint16 printer_vendor;
    uint16 printer_product;
    uint16 modem_vendor;
    uint16 modem_product;

    public bool found_printer = false;
    public bool found_modem = false;

    public UsbChecker(string printer, string modem) {
        LibUSB.Context.init(out context);
        printer.scanf("%4x:%4x", &printer_vendor, &printer_product);
        modem.scanf("%4x:%4x", &modem_vendor, &modem_product);
    }

    public void update() {
        LibUSB.Device[] list;
        ssize_t count = context.get_device_list(out list);

        found_printer = false;
        found_modem = false;
        for (ssize_t i = 0; i < count; ++i) {
            LibUSB.DeviceDescriptor desc;
            if (list[i].get_device_descriptor(out desc) != 0)
                continue;
            if (desc.idVendor == printer_vendor && desc.idProduct == printer_product)
                found_printer = true;
            if (desc.idVendor == modem_vendor && desc.idProduct == modem_product)
                found_modem = true;
        }
    }
}

int main(string[] args) {
    MainLoop loop = new MainLoop();

    Cancellable cancellable = new Cancellable();

    var url = args[1];
    var token = args[2];
    var fb_dev = args[3];
    var printer_dev = args[4];
    var modem_dev = args[5];

    try {
        Manager manager = new Manager(url, token);
        Display display = new Display(fb_dev);
        ModemChecker modem_checker = new ModemChecker();

        UsbChecker usb_checker = new UsbChecker(printer_dev, modem_dev);

        uint32 display_slots_count = display.slots_count();

        manager.connection_state_changed.connect((t, connected) => {
            display.update_cloud(connected);
            if (connected)
                print("Connected to %s...\n", url);
            else
                print("Disconnected\n");
        });
        manager.cancel_all_jobs();
        manager.run.begin(cancellable, (obj, res) => {
            manager.run.end(res);
            loop.quit();
        });
        Timeout.add(250, () => {
            modem_checker.update(cancellable);
            usb_checker.update();
            manager.update_cups_jobs();

            if (modem_checker.signal_level < 0 && usb_checker.found_modem)
                display.update_antenna(0);
            else
                display.update_antenna(modem_checker.signal_level);

            display.update_connection(modem_checker.connected);
            display.update_printer(usb_checker.found_printer);

            uint32 i = 0;
            manager.foreach_job((job) => {
                display.update_slot(i++, job.display_icon(), (uchar) (job.id % 100), job.message);
            });
            while (i < display_slots_count)
                display.update_slot(i++, '\0');

            return true;
        });
        Timeout.add(5000, () => {
            manager.send_dummy();
            return true;
        });
    } catch (GLib.Error e) {
        print("ERROR: %s\n", e.message);
    }

    loop.run ();
    return 0;
}
