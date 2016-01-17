// http://projects.robinbowes.com/flac2mp3/trac/ticket/30

sub convertReplayGainToSoundCheck {
	my ($gain, $peak) = @_;
	if ( $gain =~ /(.*)\s+dB$/ ) {
    		$gain = $1;
	}
	my @soundcheck;
	@soundcheck[0,2] = ( gain2sc($gain, 1000), gain2sc($gain, 2500) );
	@soundcheck[1,3] = @soundcheck[0,2];
	$soundcheck[6] = replayGainPeakToSoundCheckPeak($peak);
	$soundcheck[7] = $soundcheck[6];
	# bogus values for now -- however, these don't seem to be used AFAIK
	@soundcheck[4,5,8,9] = ('00024CA8', '00024CA8', '00024CA8', '00024CA8');
	return @soundcheck;
}

sub gain2sc {
	my ($gain, $base) = @_;
	my $result = round(pow(10, -$gain / 10) * $base);

        if ($result > 65534) {
                $result = 65534;
        }

        return decimalToASCIIHex($result);
}

sub round {
	my $number = shift;
	return int($number + .5 * ($number <=> 0));
}

sub decimalToASCIIHex {
	return sprintf("%08X", shift);
}

sub replayGainPeakToSoundCheckPeak {
	my $rgPeak = shift;
	# bogus values for now -- however, these don't seem to be used AFAIK
	return "00007FFF";
}

// use strtod
convertReplayGainToSoundCheck(double gain, double peak)
{
	char soundcheck[10][8];
	
	gain2sc(soundcheck[0]
}


/*
Version: 0.1

Usage:
ipodrg.exe file.mp3 a      for Album Gain
ipodrg.exe file.mp3 t      for Track Gain

If it finds a Replay Gain tag AND a Sound Check tag,
it alters the Sound Check tag to match the Replay Gain value.

This is mostly for people that still use iTunes (like me) but also want to use Replay Gain.

Thanks to Otto42, Mike Giacomelli, Aero and anyone I forgot at Hydrogenaudio for clearing out the converion formula.

Comments: johan.debock@gmail.com

Copyright (c) 2006 Johan De Bock
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int main(int argc, char* argv[]){
	if ( argc!=3) {
		printf("Wrong command line, enter:\n%s file.mp3 a    for Album Gain\n%s file.mp3 t     for Track Gain\n",argv[0],argv[0]); 
		exit(1);
	}

	FILE* in;
	if ( ( in=fopen(argv[1],"r+b") )==NULL ) {
		printf("ERROR: Could not open input file!\n"); 
		exit(1); 
	}
	char rgc=argv[2][0];
	if (rgc!='t' && rgc!='a') { 
		printf("Wrong command line, enter:\n%s file.mp3 a      for Album Gain\n%s file.mp3 t      for Track Gain\n",argv[0],argv[0]); 
		exit(1); 
	}

	char buf[46];
	char tg_utf[46]={0x72,0x00,0x65,0x00,0x70,0x00,0x6C,0x00,0x61,0x00,0x79,0x00,0x67,0x00,0x61,0x00,0x69,0x00,0x6E,0x00,0x5F,0x00,0x74,0x00,0x72,0x00,0x61,0x00,0x63,0x00,0x6B,0x00,0x5F,0x00,0x67,0x00,0x61,0x00,0x69,0x00,0x6E,0x00,0x00,0x00,0xFF,0xFE};
	char tg_iso[22]={0x72,0x65,0x70,0x6C,0x61,0x79,0x67,0x61,0x69,0x6E,0x5F,0x74,0x72,0x61,0x63,0x6B,0x5F,0x67,0x61,0x69,0x6E,0x00};
	char ag_utf[46]={0x72,0x00,0x65,0x00,0x70,0x00,0x6C,0x00,0x61,0x00,0x79,0x00,0x67,0x00,0x61,0x00,0x69,0x00,0x6E,0x00,0x5F,0x00,0x61,0x00,0x6C,0x00,0x62,0x00,0x75,0x00,0x6D,0x00,0x5F,0x00,0x67,0x00,0x61,0x00,0x69,0x00,0x6E,0x00,0x00,0x00,0xFF,0xFE};
	char ag_iso[22]={0x72,0x65,0x70,0x6C,0x61,0x79,0x67,0x61,0x69,0x6E,0x5F,0x61,0x6C,0x62,0x75,0x6D,0x5F,0x67,0x61,0x69,0x6E,0x00};

	printf("Filename: %s\n",argv[1]);

	int rgfound=0;
	if (rgc=='t') {
		while( fread(&buf[45],1,1,in) ) {
			if (memcmp(&buf[24],tg_iso,22)==0) { rgfound=1; break; }
			if (memcmp(buf,tg_utf,46)==0) { rgfound=2; break; }
			int i;
			for (i=0;i<45;i++) buf[i]=buf[i+1];
		}
	} else {
		while( fread(&buf[45],1,1,in) ) {
			if (memcmp(&buf[24],ag_iso,22)==0) { rgfound=1; break; }
			if (memcmp(buf,ag_utf,46)==0) { rgfound=2; break; }
			int i;
			for (i=0;i<45;i++) buf[i]=buf[i+1];
		}
	}

	char string[8];
	if (rgfound==1) {
		int i=0;
		char cbuf[1];
		while ( fread(cbuf,1,1,in) ) {	
			if (cbuf[0]==' ') break;
			string[i]=cbuf[0];
			i++;
		}
		string[i]='\0';	
	}
	else if (rgfound==2) {
		int i=0;
		char cbuf[1];
		while ( fread(cbuf,1,1,in) ) {
			if (cbuf[0]==' ') break;
			if (i%2==0) string[i/2]=cbuf[0];
			i++;
		}
		string[i/2]='\0';	
	}
	else { 
		printf("ERROR: Replay Gain tag not found!\n"); 
		exit(1); 
	}

	double rg=atof(string);
	if (rgc=='t')
		printf("Track Gain: %f\n",rg);
	else printf("Album Gain: %f\n",rg);

	unsigned int sc=(unsigned int)(pow(10,rg*-0.1)*1000);

	printf("Sound Check: %08X\n",sc);

	fflush(in); rewind(in);

	int scfound=0;
	char buf2[11];
	char itun1[10]={0x69,0x54,0x75,0x6E,0x4E,0x4F,0x52,0x4D,0x00,0x20};
	char itun2[11]={0x69,0x54,0x75,0x6E,0x4E,0x4F,0x52,0x4D,0x3A,0x20,0x20};
	while( fread(&buf2[10],1,1,in) ) {
		if (memcmp(&buf2[1],itun1,10)==0) { scfound=2; break; }
		if (memcmp(buf2,itun2,11)==0) { scfound=1; break; }
		int i;
		for (i=0;i<10;i++) buf2[i]=buf2[i+1];
	}
	if (scfound==0) {
		printf("ERROR: Sound Check tag not found!\n"); 
		exit(1); 
	}
	if (scfound==1) { 
		printf("ERROR: Invalid Sound Check tag! Remove the Sound Check tag with Foobar and reapply it with iTunes!\n"); 
		exit(1); 
	}

	long pos=ftell(in); fflush(in); fseek(in,pos,SEEK_SET);

	int j;
	for (j=0;j<10;j++) {
		fprintf(in,"%08X",sc);
		fseek(in,1,SEEK_CUR);
	}

	fclose(in);
}
