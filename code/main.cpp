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
#include "CTAPacketBuffer.h"
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

	printf ("Hello from task %d on %s!\n", taskid, hostname);
	if (taskid == MASTER)
	   printf("MASTER: Number of MPI tasks is: %d\n",numtasks);

	try {

		cout << "Load configurations..." << endl;


		RTATelem::CTAPacketBuffer buff(ctarta + "/share/rtatelem/rta_fadc.stream", argv[1]);


		/*
		RTATelem::CTAPacket packet(ctarta + "/share/rtatelem/rta_fadc.stream");

		RTATelem::CTACameraTriggerData trtel(ctarta + "/share/rtatelem/rta_fadc.stream");

		cout << "Source Packet Stream config file: " << packet.getPacketStreamConfig() << endl;
		*/
		cout << "Load packets..." << endl;
		int bufferSize = 500;
		//buff.load(0, 500);
		//bufferSize = buff.size();

		cout << "Start EBsim..." << endl;

		double starttime, endtime;

		dword size = 0;
		size = 128000;

		byte rawPackets[size];
		byte rawPacketr[size];

		clock_t t = clock();
		clock_t t2;
		starttime = MPI_Wtime();
		for(int i=0; i<bufferSize; i++) {

			if (send_type == 0){
					int dest, source;

					if(taskid == MASTER) {
							dest = 1;
							//rawPackets = buff.pop();

							//dword sizep = packet.getInputPacketDimension(rawPackets);
							//int type = -1;
							//type = packet.getInputPacketType(rawPackets);
							//cout << "Packets #" << i << " size: " << sizep << " byte. type: " << type << endl;

							//size = packet.getInputPacketDimension(rawPacket);
							//cout << i << " S" << (int) rawPackets[0] << " - " << (int) rawPackets[1] << endl;
							MPI_Send(rawPackets, size, MPI_UNSIGNED_CHAR, dest, 1, MPI_COMM_WORLD); //AB

							t2 = clock();
							//printf("send npacket: %d\n", npacket);
					} else {
							//source = MASTER;
							MPI_Recv(&rawPacketr, size, MPI_UNSIGNED_CHAR, MASTER, 1, MPI_COMM_WORLD, &status); //AB
							//endtime = MPI_Wtime();
							t2 = clock();
							//cout << i << " R" << (int) rawPacketr[0] << " - " << (int) rawPacketr[1] << endl;

							/*
							dword sizep = -1;
							sizep = packet.getInputPacketDimension(rawPacketr);
							int type = -1;
							type = packet.getInputPacketType(rawPacketr);
							cout << "Packetr #" << i << " size: " << sizep << " byte. type: " << type << endl;
							switch(type) {
							case 1:
								//trtel.setStream(rawPacketr);
								//cout << "Index Of Current Triggered Telescope " << (long) trtel.getIndexOfCurrentTriggeredTelescope() << endl;
								break;

							};
							*/
					}
			}
			endtime = MPI_Wtime();
			/*byte* rawPacket = buff.pop();
			dword size = 0;
			size = packet.getInputPacketDimension(rawPacket);
			int type = -1;
			type = packet.getInputPacketType(rawPacket);
			switch(type) {
			case 1:
				trtel.setStream(rawPacket);
				//cout << "Index Of Current Triggered Telescope " << (long) trtel.getIndexOfCurrentTriggeredTelescope() << endl;
				break;

			};
			cout << "Packet #" << i << " size: " << size << " byte. type: " << type << endl;
			*/
		}


		printf("%d - That took %f seconds\n",taskid, endtime-starttime);
		printf("%d - %f MB/s\n", taskid, ((size * bufferSize * 1)/(endtime-starttime)) / 1000000);
		printf("Clock resolution: %f\n",MPI_Wtick());

		t = t2 - t;
		cout << taskid << " - It took me " << t << " clicks (" << ((float)t)/CLOCKS_PER_SEC << " seconds)" << endl;
		cout << taskid << " - END" << endl;

		MPI_Finalize();

	} catch(PacketException* e)
	{
	        cout << e->geterror() << endl;
	}

}
