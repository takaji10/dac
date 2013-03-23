#ifndef _IR_REMOTE_H
#define _IR_REMOTE_H

#define REMOTEPIN 7

#define remote_menu	0x77EC007C
#define remote_enter	0x77E1??7C, 0x77E1??7C
#define remote_up	0x77E1507C
#define remote_down	0x77E1307C
#define remote_left	0x77E1907C
#define remote_right	0x77E1607C
#define remote_play	0x77E1??7C, 0x77E1A07C
#define remote_pause	remote_play

int getIRkey();

#endif
