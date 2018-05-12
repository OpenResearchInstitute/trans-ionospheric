#set -x
# This is a sample of how you can extract the first
# frame from each bling gif and convert it to a 20x20 gif
# for conversion to an icon.
# Make a list of your file base naems, without the gif extensions

for file in file1 file2 etc etc1
do
  echo "processing $file.gif"
  convert "$file"'.gif[0]' $file-frame.gif
  gifsicle --resize 20x20 "$file"-frame.gif > "$file"-ico.gif
  rm "$file"-frame.gif
done
