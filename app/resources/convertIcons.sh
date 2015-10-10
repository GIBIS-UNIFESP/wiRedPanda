find . -type f -name '*.svg' | while read file; do
  inkscape -z $file -e ${file%.*}.png;
done;
