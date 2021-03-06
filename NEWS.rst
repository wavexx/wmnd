WMND 0.4.17
-----------

	* Minor documentation/packaging fixes.


WMND 0.4.16
-----------

	* Fixed build failure on FreeBSD 8.1.


WMND 0.4.15
-----------

	* Fixed build failure of the SNMP driver.


WMND 0.4.14
-----------

	* Fixed build on newer GLIBC versions.
	* Remove spurious black line at the bottom of the pixmap
	  (Thanks to Mikael Magnusson)


WMND 0.4.13
-----------

	* Fixed crash due to buffer overflow on 64bit systems.
	* Fixed incorrect display on linux for devices having similar names.
	* Fixed FreeBSD driver on 64bit systems.
	* Added OpenSolaris support for broken kstat interfaces.
	* Enhanced trend support with live monitoring (even with multiple
	  interfaces at once), customizable flags and history.
	  Trend support is now on by default (Rev #68 required).
	* Removed support for "inexact timing".
	* The enhanced dummy driver is now the default.


WMND 0.4.12
-----------

	* Executed processes are no longer left as zombies.
	* Solaris 7 build fixes and kstat enhancements.
	* NetBSD ioctl support.
	* Maintainers warning: some flag names changed in 'configure'.


WMND 0.4.11
-----------

	* Édition spécial Parisienne!
	* Added support for history zoom/inspection through trend.
	  See README for more details.


WMND 0.4.10
-----------

	* Fixed generic_snmp for old snmpd by probing both ifName and
	  ifDesc. --enable-snmp-descr is no longer needed.


WMND 0.4.9
----------

	* The auto-generated wmndrc now only includes changed values.
	* Added a geometry flag (-g, requested by Johannes Middeke).


WMND 0.4.8
----------

	* linux_proc supports multiple interfaces on the command
	  line (integrated patch wmnd-0.4.7-lxext, requested by
	  Jesper Anderson).
	* Mouse button actions now supports several %-escapes so you
	  can send active interface parameters to your scripts.
	  (requested by Jesper Anderson).
	* Support for mouse wheel (integrated patch wmnd-0.4.7-wheel
	  by Noberasco Michele).
	* Support for fixed scales for the bytes modes (requested by
	  Jeff Greenfield a looong time ago).


WMND 0.4.7
----------

	* WMND statistics are more accurate now with a finer
	  time-based method (should remove a 10% error). You can switch
	  back to the old method (produces a smoother graph if you
	  don't mind about stats) with --enable-inexact-timing.
	* Packets mode is no longer smoothed (quite useless) with -o.
	* Leds blinks again even when smoothing is enabled.
	* Fixed a bug in the smooth function (could produce an empty
	  graph depending on the compiler used).
	* The 'whole history' max stats should now report real
	  values instead of an huge nonsense.
	* You can now change the title/class name of WMND using -n.
	  This is useful on wharf (afterstep) to not swallow multiple
	  instances into a single dock.


WMND 0.4.6
----------

	* Fixed a typo in the configure script that could cause some
	  shells to interrupt the configuration.
	* generic_snmp will now use ifName instead of ifDescr, which
	  is usually shorter. Again, issues in the README.


WMND 0.4.5
----------

	* Added a new IF-MIB snmp driver! You can now monitor local
	  and remote snmp interfaces a-la MRTG, but in realtime! Read
	  CAREFULLY the README about issues with this driver.
	* Some packaging 'hints' into the README.


WMND 0.4.4a
-----------

	* Fixed a bug into the solaris_fpppd driver (won't compile).


WMND 0.4.4
----------

	* WMND can now be quiet (-q/Q or quiet into ~/.wmndrc).
	* Fixed some parts of the configure script. Some options have
	  changed their names:

	    --without-dummy_driver is now --without-dummy-driver,
	    --with-display_modes is now --with-display-modes

	* WMND is now fully packaged with 'make dist' (fixes some bugs
	  in the installation procedure). WMND now requires autoconf 5
	  and automake 1.7 for regenerating the configure script.
	* Tired of the boring dummy driver? Enable the sine-o-matic
	  bandwidth generator with --enable-sine-dummy! Then, experience
	  it with:

	    wmnd -D testing_dummy -s 1

	* The --enable-debug switch should now work again.
	* I now maintain WMND with cvs, so I use cvs2cl to produce the
	  ChangeLog directly from the repository. Old entries are moved
	  to the ChangeLog.0 file.
	* The linux_proc driver can be forced to monitor devices that
	  are actually offline (useful for ppp interfaces).
	* New display mode: lines (trend-like behavior). Mainly useful
	  with a low smoothing factor and a fast scroll speed.
	* Added the smoothing factor thing to clear-up the graph while using a
	  fast scroll speed and the lines mode. Read the docs for more
	  informations.
	* Fixed average sampling (-S) for multiple devices and values != 1
	* Added a new IRIX Performance Co-Pilot driver!
	* Re-touched the "charts" pixmap, in order to use less colors
	  (PseudoColor users will appreciate this). I find it nicer anyway :)
	* The timer now works for *every* device. linux_proc users can
	  now use it for ppp links.


WMND 0.4.3
----------

	* Fixed a minor bug into the freebsd_sysctl driver that would
	  crash WMND under some circumstances.
	* Added the new -S flag for slowing down the rate meter.
	* Minor code/spell corrections.


WMND 0.4.2
----------

	* added the missing -lkstat flag to the solaris kstat driver.
	* the graph scrolling speed is now specified in tenth of seconds
	  (so update your .wmndrc accordingly). wmnet mode under wmnd is
	  now identical to the wmnd dockapp (except for colors). The needle
	  mode is also much nicer with a fast refresh.
	* big improvements around the code, now wmnd compiles fine with
	  a c++ compiler in strict mode.
	* man page update (Thanks Arthur Korn).
	* New visual mode: mgraph
	* Drivers can be now selected manually again
	* Tune-up of the freebsd_sysctl driver


WMND 0.4.1
----------

	* fixed the solaris fpppd driver


WMND 0.4.0
----------

	* auto-detection stuff
	* remove visualization modes from configure
	* FreeBSD is now supported
	* general code cleanup
	* package reorganization
