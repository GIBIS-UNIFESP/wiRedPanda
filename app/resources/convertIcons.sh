# find . -type f -name '*.svg' | while read file; do
#   inkscape -z $file -e ${file%.*}.png;
# done;

for size in 26 32 48 64 128; do
  inkscape -z wpanda.svg -e icons/${size}x${size}/wpanda.png -h ${size};
done;
