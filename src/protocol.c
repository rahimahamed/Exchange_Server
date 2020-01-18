#include "protocol.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload){
    if(write(fd, hdr, sizeof(BRS_PACKET_HEADER)) ==-1)
        return -1;
    if(hdr->size != 0){
        if(write(fd, payload, hdr->size) == -1)
            return -1;
    }
    return 0;
}

int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp){
    // //CONVERT MULTI-BYTE FIELDS IN PACKET TO HOST BYTE ORDER FROM NETWORK BYTE ORDER

    //BLOCK UNTIL WE READ HEADER
    int r;
    r = read(fd, hdr, sizeof(BRS_PACKET_HEADER));
    if(r == -1)
        return -1;
    //IF EOF
    if(r == 0)
        return -1;

    //CONVERT HEADER TO HOST BYTE ORDER
    hdr->size = ntohs(hdr->size);
    hdr->timestamp_sec = ntohl(hdr->timestamp_sec);
    hdr->timestamp_nsec = ntohl(hdr->timestamp_nsec);

    //IF PAYLOAD EXISTS BLOCK UNTIL WE READ PAYLOAD
    if(hdr->size != 0){
        *payloadp = malloc(hdr->size);
        r = read(fd, *payloadp, hdr->size);
        if(r == -1)
            return -1;
        //IF EOF
        if(r == 0)
            return -1;
    }

    //CONVERT PAYLOAD TO HOST BYTE ORDER
    if(hdr->size != 0 || hdr->type != BRS_NACK_PKT || hdr->type != BRS_NO_PKT || hdr->type != BRS_STATUS_PKT){
        if(hdr->type == BRS_DEPOSIT_PKT || hdr->type == BRS_WITHDRAW_PKT)
            ((struct brs_funds_info *)*payloadp)->amount = ntohl(((struct brs_funds_info *)*payloadp)->amount);
        if(hdr->type == BRS_ESCROW_PKT || hdr->type == BRS_RELEASE_PKT)
            ((struct brs_escrow_info *)*payloadp)->quantity = ntohl(((struct brs_escrow_info *)*payloadp)->quantity);
        if(hdr->type == BRS_BUY_PKT || hdr->type == BRS_SELL_PKT){
            ((struct brs_order_info *)*payloadp)->quantity = ntohl(((struct brs_order_info *)*payloadp)->quantity);
            ((struct brs_order_info *)*payloadp)->price = ntohl(((struct brs_order_info *)*payloadp)->price);
        }
        if(hdr->type == BRS_CANCEL_PKT)
            ((struct brs_cancel_info *)*payloadp)->order = ntohl(((struct brs_cancel_info *)*payloadp)->order);
        if(hdr->type == BRS_ACK_PKT){
            ((struct brs_status_info *)*payloadp)->balance = ntohl(((struct brs_status_info *)*payloadp)->balance);
            ((struct brs_status_info *)*payloadp)->inventory = ntohl(((struct brs_status_info *)*payloadp)->inventory);
            ((struct brs_status_info *)*payloadp)->bid = ntohl(((struct brs_status_info *)*payloadp)->bid);
            ((struct brs_status_info *)*payloadp)->ask = ntohl(((struct brs_status_info *)*payloadp)->ask);
            ((struct brs_status_info *)*payloadp)->last = ntohl(((struct brs_status_info *)*payloadp)->last);
            ((struct brs_status_info *)*payloadp)->orderid = ntohl(((struct brs_status_info *)*payloadp)->orderid);
            ((struct brs_status_info *)*payloadp)->quantity = ntohl(((struct brs_status_info *)*payloadp)->quantity);
        }
        if(hdr->type == BRS_BOUGHT_PKT || hdr->type == BRS_SOLD_PKT || hdr->type == BRS_POSTED_PKT || hdr->type == BRS_CANCELED_PKT || hdr->type == BRS_TRADED_PKT){
            ((struct brs_notify_info *)*payloadp)->buyer = ntohl(((struct brs_notify_info *)*payloadp)->buyer);
            ((struct brs_notify_info *)*payloadp)->seller = ntohl(((struct brs_notify_info *)*payloadp)->seller);
            ((struct brs_notify_info *)*payloadp)->quantity = ntohl(((struct brs_notify_info *)*payloadp)->quantity);
            ((struct brs_notify_info *)*payloadp)->price = ntohl(((struct brs_notify_info *)*payloadp)->price);
        }
    }

    return 0;
}
