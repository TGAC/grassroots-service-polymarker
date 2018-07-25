Polymarker Service {#polymarker_service_guide}
==============

The [Polymarker](http://polymarker.tgac.ac.uk/) Service is an automated bioinformatics pipeline for SNP assay development which increases the probability of generating homoeologue-specific assays for polyploid wheat.

## Installation

To build this service, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. If you are running Polymarker  on the host machine, you will also need the [Polymarker application](https://github.com/TGAC/bioruby-polyploid-tools) and its dependencies installed.

The files to build the Polymarker service are in the ```build/<platform>``` directory. 

### Linux

If you enter this directory 

~~~
cd build/linux
~~~


you can then build the service by typing

~~~
make all
~~~

and then 

~~~
make install
~~~

to install the service into the Grassroots system where it will be available for use immediately.

## Server Configuration

Each of the three services listed above can be configured by files with the same names in the ```config``` directory in the Grassroots application directory, *e.g.* ```config/Polymarker service```

 * **working_directory**: This is the directory where are any input, output and log files created by the Polymarker Services. This directory must be writeable by the user running the Grassroots Server. For instance, the httpd server is often run as the daemon user.
 * **index_files**: This is an array of objects giving the details of the available databases. The objects in this array have the following keys:
    * **sequence**:  This is the name to show to the user for this database. 
    * **fasta**: This is the database value that the Polymarker service will use to search against.
 * **tool**: This determines how the Polymarker search will be run and currently has the following options:
    * **system**: This will be run using the executable specified by *tool_executable* asynchronously on the host machine. This is the default *tool* option.
 * **tool\_executable**: This is the path to the executable used to perform the searches. 


An example configuration file for the Polymarker service which would be saved as the ```<Grassroots directory>/config/Polymarker service``` is:

~~~{.json}
{
	"icon_uri": "http://localhost:8080/grassroots/images/Polymarker%20service",
	"tool": "system",
	"tool_executable": "/opt/bioruby-polyploid-tools/bin/polymarker_grassroots.rb",
	"working_directory": "/opt/grassroots-0/working_directory/polymarker",
	"index_files": [{
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/Chinese_spring_TGAC_v1_arm-classified.fasta",
		"Sequence": "Chinese Spring"
	}, {
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/Cadenza_EI_v1_arm-classified.fasta",
		"Sequence": "Cadenza"
	}, {
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/Triticum_aestivum_CS42_TGACv1_scaffold.annotation.gff3.pep.fa",
		"Sequence": "Triticum aestivum CS42 TGACv1 peptide"
	}, {
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/Triticum_aestivum_CS42_TGACv1_scaffold.annotation.gff3.cds.fa",
		"Sequence": "Triticum aestivum CS42 TGACv1 cds"
	}, {
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/Triticum_aestivum_CS42_TGACv1_scaffold.annotation.gff3.cdna.fa",
		"Sequence": "Triticum aestivum CS42 TGACv1 cdna"
	}, {
		"Fasta": "/opt/grassroots-0/grassroots/extras/blast/databases/wwt.fa",
		"Sequence": "Worldwide Wheat Transcriptome"
	}]
}
~~~
