# How Bling works

## Categories of Bling Video Animations

There are two (well, three) categories of bling.

One I’ll call ‘regular’ bling, and that’s a mode that appears in the bling menus on the badge.

The second is a “backer’ bling, and that’s basically the same except it’s in a different menu location.

The third is “BYOB” or “Bring your own Bling”, which can be added by just copying files to the SD card. Those files appear in the menu under a ‘custom bling’ mode and can be launched from there.

## File Formats

## Implementation

The file formats are the same for all these modes and you can test bling by using BYOB before adding it to the menus, which requires source code changes to the badge code base. There’s a google doc titled “Menu Layout” in the same folder as this doc that has the Bender badge menu hierarchy.

In addition to the gif file with the animation for the LCD (.RAW extension), each bling mode ~may~ also have a custom callback associated with it in the menu which defines the LED flashing patterns. If there’s not a custom callback, one is used that maps the LED flashing and colors to the frames in the gif file. I don’t completely understand this yet, I haven’t looked at it much. I do know that we’ll need to change the mapping because of the differences in location between the original Bender eye LEDs and the remixed badges.

And lastly, there are “.RGB” files, which is a 4x3 sized file in the same format, to be applied to the LEDs along with the BYOB modes in “custom bling”.

This page describes how to prep the gif files and place them on the SD card for BYOB.

https://hackaday.io/project/19121-andxor-dc25-badge/log/58327-secret-component-feature-sd-card-byob

Menu entries also need an icon (.ICO) and a preview (.PRV) file in addition to the .RAW file.

## Preparing Bling Selected from Menus

### File Formats

The RAW, PRV, and ICO files are all in rawvideo rgb565be format, which is 16 bits per pixel - 5,6,5 for R,G,B. Big Endian. This file format matches the data expected by the LCD display and allows DMA of the file data directly to the display.

PRV and ICO files only contain a single frame

RAW files are 128 x 128 pixels
PRV files are 101 x 112 pixels
ICO files are 20 x 20 pixels and all the current ones are two-color (black and white)
File Locations on the SD Card

Typically, RAW files are in the “BLING” directory
Typically, PRV and ICO files are in the “MENU” directory

### Workflow

Workflow for preparing all the files for a bling mode

This assumes that you want to create the .RAW file and .PRV preview file from an animated gif, and the icon from a separate file. There are sample shell scripts in the  imagetools directory to make it easier to use ffmpeg to do the conversions. They may need customization for your use case.

Start with an animated gif, which must be at least 128x128 pixels.

If needed, resize it to 128x128 using [this ezgif online tool](https://ezgif.com/resize/).
You can use that same tool to crop it to 128x128 if you want.

Or crop it using [this gifgifs tool](http://gifgifs.com/crop/).
Create one file that’s 128x128 and one that’s 101x112.

You also have to make an image that’s 101x112, and is the lower right part of the image, for the preview (.PRV) file. the ezgif tool won’t let you do that, but the gifgifs tool will. 

When you’re done you’ll have two files for each animation: one file that’s 128x128 and one that’s 101x112.

Now use the tools in the imagetools directory to convert them.

Generate the RAW file:

```
./gif2raw input-128x128.gif outfile.RAW
```

Generate the PRV file (uses the first frame of the gif for the preview):

```
./gif2prv input-101x112.gif outfile.PRV
```

Create a 20x20 pixel icon using the tools of your choice. For consistency, make it black and white. Export/save it as a gif file.

You can extract the first frame of an animated gif like this if you have ImageMagick installed:

```
convert 'filename.gif[0]' filename-frame.gif
```

You can resize that gif to 20x20 pixels using gifsicle (install it first) like this:

```
gifsicle --resize 20x20 filename-frame.gif > filename-ico.gif
rm filename-frame.gif
```

Generate the ICO file:

```
./gif2prv input-20x20.gif outfile.ICO
```

