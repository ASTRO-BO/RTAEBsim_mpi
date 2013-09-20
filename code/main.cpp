/***************************************************************************
                          main.cpp  -  description
                             -------------------
    copyright            : (C) 2013 Andrea Bulgarelli
                               2013 Andrea Zoli
    email                : bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include "CTACameraTriggerData.h"
#include "CTACameraPedestal.h"
#include "CTAPacketBufferV.h"
#include <time.h>
#include <mpi.h>

using namespace std;

#define  MASTER         0
#define MPI_WTIME_IS_GLOBAL 0

int main(int argc, char *argv[])
{
	string ctarta;
	const char* home = getenv("CTARTA");

	if (!home) {
		cerr << "CTARTA environment variable is not defined." << endl;
		return 0;
	}
	ctarta = home;

	if(argc != 2) {
		cerr << "Please, provide the .raw" << endl;
		return 0;
	}

	int  numtasks, taskid, len;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int  partner;
	int  send_type = 0;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Get_processor_name(hostname, &len);

	clock_t t[numtasks];
	clock_t t2[numtasks];
	double starttime[numtasks], endtime[numtasks];
	/*
	printf ("Hello from task %d on %s!\n", taskid, hostname);
	if (taskid == MASTER)
	   printf("MASTER: Number of MPI tasks is: %d\n",numtasks);
	*/
	try {

		//cout << "Load configurations..." << endl;


		RTATelem::CTAPacketBufferV buff(ctarta + "/share/rtatelem/rta_fadc.stream", argv[1]);

		RTATelem::CTACameraTriggerData trtel(ctarta + "/share/rtatelem/rta_fadc.stream");

		//cout << "Source Packet Stream config file: " << packet.getPacketStreamConfig() << endl;

		//cout << "Load packets..." << endl;
		int bufferSize;
		buff.load(0, 50);
		bufferSize = buff.size();

		//cout << "Start EBsim..." << endl;



		dword size = 0;
		size = 128000;

		byte *rawPackets;
		byte rawPacketr[size];

		t[taskid] = clock();
		starttime[taskid] = MPI_Wtime();

		unsigned long npackets = 500000;
		unsigned long totmbs = 0;
		if(taskid == MASTER) cout << "npacket: " << npackets << endl;
		for(int i=0; i<npackets; i++) {

			if (send_type == 0){
					int dest, source;

					if(taskid == MASTER) {
							dest = 1;
							rawPackets = buff.get();

							//dword sizep = packet.getInputPacketDimension(rawPackets);
							//int type = -1;
							//type = packet.getInputPacketType(rawPackets);
							//cout << "Packets #" << i << " size: " << sizep << " byte. type: " << type << endl;

							//size = packet.getInputPacketDimension(rawPacket);
							//cout << i << " S" << (int) rawPackets[0] << " - " << (int) rawPackets[1] << endl;
							MPI_Send(rawPackets, size, MPI_UNSIGNED_CHAR, dest, 1, MPI_COMM_WORLD); //AB
							totmbs += size;


							//printf("send npacket: %d\n", npacket);
					} else {

						if(taskid == 1) {
							MPI_Recv(&rawPacketr, size, MPI_UNSIGNED_CHAR, source=MASTER, 1, MPI_COMM_WORLD, &status); //AB
							//endtime = MPI_Wtime();

							//cout << i << " R" << (int) rawPacketr[0] << " - " << (int) rawPacketr[1] << endl;


							//dword sizep = -1;
							//sizep = buff.packet.getInputPacketDimension(rawPacketr);


							if(numtasks > 2) {
								int type = -1;
								type = buff.packet.getInputPacketType(rawPacketr);
								if(type == 1) {
									dest = 2;
									MPI_Send(&rawPacketr, size, MPI_UNSIGNED_CHAR, dest, 1, MPI_COMM_WORLD);
									totmbs += size;
								}
							}
							//cout << "Packetr #" << i << " size: " << sizep << " byte. type: " << type << endl;
							/*
							switch(type) {
							case 1:
								trtel.setStream(rawPacketr);
								//cout << "Index Of Current Triggered Telescope " << (long) trtel.getIndexOfCurrentTriggeredTelescope() << endl;
								break;

							};
							*/
						}
						if(numtasks > 2 && taskid == 2){
							source = 1;
							MPI_Recv(&rawPacketr, size, MPI_UNSIGNED_CHAR, source, 1, MPI_COMM_WORLD, &status);

						}

					}
			}

		}
		endtime[taskid] = MPI_Wtime();
		t2[taskid] = clock();
		unsigned long totmb = (size * npackets * 1) / 1000000;
		printf("%d - That took %f seconds\n",taskid, endtime[taskid]-starttime[taskid]);
		if(taskid == MASTER)  cout <<  taskid << " - MB " <<  totmb  << endl;
		cout << taskid << " - REAL MB: " << totmbs / 1000000 << endl;
		printf("%d - %f MB/s\n", taskid, (totmb/(endtime[taskid]-starttime[taskid])) );
		//printf("Clock resolution: %f\n",MPI_Wtick());

		//t[taskid] = t2[taskid] - t[taskid];
		//cout << taskid << " - It took me " << t[taskid] << " clicks (" << ((float)t[taskid])/CLOCKS_PER_SEC << " seconds)" << endl;
		//cout << taskid << " - END" << endl;

		MPI_Finalize();

	} catch(PacketException* e)
	{
	        cout << e->geterror() << endl;
	}

}
