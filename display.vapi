[CCode (cheader_filename = "display.h", cname = "display_t", unref_function = "display_free")]
public class Display {
    public string name;
    public string value;

    public class Icon {
        public static Icon WARNING;
    }

    //  [CCode (cname = "cupsFreeOptions")]
    //  public static void free ([CCode (array_length_pos = 0.9)] Option[] options);


    [CCode (cname = "display_new")]
    public Display(string path);

    [CCode (cname = "display_slot_count")]
    public uint32 slots_count();
    [CCode (cname = "display_update_antenna")]
    public void update_antenna(int level);
    [CCode (cname = "display_update_connection")]
    public void update_connection(bool active);
    [CCode (cname = "display_update_cloud")]
    public void update_cloud(bool active);
    [CCode (cname = "display_update_printer")]
    public void update_printer(bool active);
    [CCode (cname = "display_update_slot")]
    public void update_slot(uint32 slot, char icon, uchar id = 0, string? message = null);
}
