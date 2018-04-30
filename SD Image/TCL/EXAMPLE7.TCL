cls;
print 0 0 "Counting on I/O Pins, press any button to exit"

set a0 0;
set a1 0;
set a2 0;
set a3 0;
set a4 0;
while {== [button_state] 0} {
	if {== 0 $a0} {
		io_write 0 LOW
	}
	if {== 1 $a0} {
		io_write 0 HIGH
	}
	if {== 0 $a1} {
		io_write 1 LOW
	}
	if {== 1 $a1} {
		io_write 1 HIGH
	}
	if {== 0 $a2} {
		io_write 2 LOW
	}
	if {== 1 $a2} {
		io_write 2 HIGH
	}
	if {== 0 $a3} {
		io_write 3 LOW
	}
	if {== 1 $a3} {
		io_write 3 HIGH
	}
	if {== 0 $a4} {
		io_write 4 LOW
	}
	if {== 1 $a4} {
		io_write 4 HIGH
	}
	
	incr a0;
	if {== $a0 2} {
		set a0 0;
		incr a1;
		if {== $a1 2} {
		set a1 0;
		incr a2;
		if {== $a2 2} {
			set a2 0;
			incr a3;
			if {== $a3 2} {
				set a3 0;
				incr a4;
				if {== $a4 2} {
					set a4 0;
					}
				}
			}
		}
	}
};
