/*
 * Code taken from with minor changes:
 * http://www.xenoscope.com/weblog/2008/09/16/how-to-monitor-parallel-port-interrupts-in-user-space-in-linux/
 * 
 * Example code from:
 * http://people.redhat.com/twaugh/parport/html/x916.html
 */
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <stropts.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
 
#define PP_DEV_NAME "/dev/parport0"
 
int main()
{
    //Get a file descriptor for the parallel port
    int ppfd;
 
    ppfd = open(PP_DEV_NAME, O_RDWR);
    if(ppfd == -1)
    {
        printf("Unable to open parallel port!\n");
        return 1;
    }
 
    //Have to claim the device
    if(ioctl(ppfd, PPCLAIM))
    {
        printf("Couldn't claim parallel port!\n");
        close(ppfd);
        return 1;
    }
 
    while(1)
    {
        //Set up the control lines for when an interrupt happens
        int int_count;
        int busy = PARPORT_STATUS_ACK | PARPORT_STATUS_ERROR;
        int acking = PARPORT_STATUS_ERROR;
        int ready = PARPORT_STATUS_BUSY | PARPORT_STATUS_ACK | PARPORT_STATUS_ERROR;
        char int_value;
 
        ioctl(ppfd, PPWCTLONIRQ, &busy);
 
        //Let ppdev know we're ready for interrupts
        ioctl(ppfd, PPWCONTROL, &ready);
 
        //Wait for an interrupt
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(ppfd, &rfds);
        if(select(ppfd + 1, &rfds, NULL, NULL, NULL))
        {
            printf("Received interrupt\n");
        }
        else
            continue; //Caught a signal
 
        //Fetch the associated data
        ioctl(ppfd, PPRDATA, &int_value);
 
        //Clear the interrupt
        ioctl(ppfd, PPCLRIRQ, &int_count);
        if(int_count > 1)
            printf("Uh oh, missed %i interrupts!\n", int_count - 1);
 
        //Acknowledge the interrupt
        ioctl(ppfd, PPWCONTROL, &acking);
        usleep(2);
        ioctl(ppfd, PPWCONTROL, &busy);
    }
 
    return 0;
}
