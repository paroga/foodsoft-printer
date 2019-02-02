#!/bin/bash

cat <<EOF > $1/bitmaps.h
#pragma once

typedef struct {
  unsigned char width;
  unsigned char height;
  const unsigned char* data;
} Bitmap;

EOF

cat <<EOF > $1/bitmaps.c
#include "bitmaps.h"
EOF


for f in ${@:2}
do
    data=`cat bitmap_$f.raw | xxd -i`

    echo "extern const Bitmap bitmap_$f;" >> $1/bitmaps.h
    cat <<EOF >> $1/bitmaps.c

static const unsigned char ${f}_data[] = {
$data
};

const Bitmap bitmap_$f = { sizeof(${f}_data) / 23, 23, ${f}_data };
EOF
done
