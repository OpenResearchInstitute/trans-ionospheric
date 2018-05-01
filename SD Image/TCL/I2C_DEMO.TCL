cls;
print 0 0 "I2C Demo, press any button to exit";

set x 0;

while {== [button_state] 0} {
	i2c_write 32 2 0 240;
	i2c_write 32 2 18 $x;
	incr x;
	delay 300;
};