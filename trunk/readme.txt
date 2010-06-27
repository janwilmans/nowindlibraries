this repository contails:

- nowind libraries
- the nwhostapp application
- the NowindInterfaceHostGUI win32 front for the commandline nwhostapp application

currently a proper unix makefile is missing, but a generated Makefile is present in nwhostapp/

external dependencies:
- libusb-dev
- 



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