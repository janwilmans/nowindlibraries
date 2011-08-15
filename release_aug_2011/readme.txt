this repository contains:

- nowind libraries
- the nwhostapp application
- the NowindInterfaceHostGUI win32 front for the commandline nwhostapp application

external dependencies:
- libusb-dev
- libftdi-devel   (sudo yum install libftdi-devel on Fedora, extract sources on windows)
- boost >1.44

Example of compiling on Fedora 13:
- make sure boost headers are in /usr/include/boost, see BOOST) below
- export BOOST_ROOT=/usr
- sudo yum -y install cmake
- sudo yum -y install libftdi-devel
- use 'cmake -G 'Unix Makefiles' to generate makefiles.
- run 'make' 

BOOST)
extract: tar --bzip2 -xf /path/to/boost_1_46_0.tar.bz2
install: 
> sudo bash
> ./bootstrap.sh --prefix=/usr
> ./bjam install
> ldconfig (will update the shared library search cache)

now close and reopen the shell.
 
http://www.boost.org/doc/libs/1_46_0/more/getting_started/unix-variants.html

to do a clean build:
> cd ~src/nowindlibraries
> sudo rm -rf CMakeFiles CMakeCache.txt
> cmake -G "Unix Makefiles"  (will locate boost through /usr/share/cmake/Modules/FindBoost.cmake)
> make

Use:
>sudo ldconfig -p | grep boost
to check the right boost-libraries are found.

> cmake -G "Unix Makefiles" --debug-output
to check where the FindBoost.cmake is located


 set(Boost_USE_STATIC_LIBS   ON)
#   set(Boost_USE_MULTITHREADED ON)
#   find_package( Boost 1.36.0 COMPONENTS date_time filesystem system ... )



<Sky_hawk> welke tools zijn er voor SCSI images?
<Quibus> NFDISK
<Quibus> en MAP32
<Quibus> tenminste, hangt er vanaf wat je wilt :-)
<Sky_hawk> wat doet map32 wat nfdisk niet doet?
<Quibus> je had 't over tools in 't algemeen
<Quibus> map32 is dus de partitie-mapper (die ik zo graag voor Nowind wil hebben)
<Quibus> nfdisk is dus de partitioner							 

// NDISK -> http://members.chello.nl/m.delorme/


<Sky_hawk> ok
<Sky_hawk> probeer nog eens
<Sky_hawk> ik had ignore aan staan
<Sky_hawk> thanks
<Sky_hawk> wat is de syntax van map32
<Sky_hawk> ?
<Quibus> Usage : MAP [-<s|S>] <drive> <targetID> <partitionnumber>
<Quibus>           -s|S     : silent mode
<Quibus>           drive    : A...H
<Quibus>           targetID : 0...7
<Quibus>           part.no. : 1...32
<Quibus>         MAP -<l|L> [targetID]
<Quibus>           -l|L     : list partitions
<Quibus>           targetID : 0...7
<Sky_hawk> wat is het target?
<Quibus> je kunt meerdere SCSI devices in je systeem hebben
<Sky_hawk> ok
<Quibus> Die hebben een ID
<Sky_hawk> makkelijk 
<Quibus> pure noodzaak
<Quibus> Als je een 1GB disk hebt, met 32 partities van 32MB, dan heb je zo'n tool hard nodig
<Quibus> Voor IDE heb je IDEPAR:
<Quibus> Partition changer for the Sunrise MSX ATA/IDE interface
<Quibus> Usage: IDEPAR idedrive[:]=partition [/M or /S]
<Quibus> Defaults: idedrive : current drive
<Quibus>           partition: 0
<Quibus>           device   : same (master/slave) as the idedrive
<Quibus> Examples:
<Quibus> IDEPAR 10     :engage partition 10 on the current drive
<Quibus> IDEPAR B:=5   :engage partition 5 on drive B:
<Quibus> IDEPAR C=2 /S :engage partition 2 of slave device on drive C:
<Quibus> IDEPAR C: /M  :engage partition 0 of master device on drive C:
