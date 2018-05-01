cls
print 0 0 "I2C RW Demo, press   any button to exit";
print 0 40 "RTC Time"

set x 0

while {== [button_state] 0} {
	i2c_write 32 2 0 240
	i2c_write 32 2 18 $x
	incr x
	
	i2c_write 104 1 0
	i2c_read 104 7
	
	set s [i2c_get_byte 0]
	set m [i2c_get_byte 1]
	set h [i2c_get_byte 2]
	
	set ss [/ $s 16]
	set sd [- $s [* $ss 16]]
	
	set mm [/ $m 16]
	set md [- $m [* $mm 16]]

	set hx [/ $h 32]
	set h [- $h [* $hx 32]]
	set hh [/ $h 16]
	set hd [- $h [* $hh 16]]
		
	fill_rect 0 55 64 15 0
	print 0 55 $hh
	print 8 55 $hd
	print 16 55 ":"
	print 24 55 $mm
	print 32 55 $md
	print 40 55 ":"
	print 48 55 $ss
	print 56 55 $sd
}
