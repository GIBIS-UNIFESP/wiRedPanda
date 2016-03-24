find . -type f -name '*.svg' | while read file; do
  inkscape -z $file -e ${file%.*}.png;
done;

for size in 26 32 48 64 128; do
  mkdir -p icons/${size}x${size}
  inkscape -z wpanda.svg -e icons/${size}x${size}/wpanda.png -h ${size};
  inkscape -z pandaFile.svg -e icons/${size}x${size}/wpanda-file.png -h ${size};
done;
