#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netdb.h>

# define SET_LI(packet, li)(uint8_t)(packet.li_vn_mode |= (li << 6))
# define SET_VN(packet, vn)(uint8_t)(packet.li_vn_mode |= (vn << 3))
# define SET_MODE(packet, mode)(uint8_t)(packet.li_vn_mode |= (mode << 0))

typedef struct{
    uint8_t li_vn_mode;
    uint8_t stratum; 
    uint8_t poll; // E i g h t b i t s . Maximum i n t e r v a l b e t w e e n s u c c e s s i v e m e s s a g e s .
    uint8_t precision; // E i g h t b i t s . P r e c i s i o n o f t h e l o c a l c l o c k .
    uint32_t rootDelay; // 32 b i t s . T o t a l round t r i p d e l a y t i m e .
    uint32_t rootDispersion; // 32 b i t s . Max e r r o r a l o u d f r o m p r i m a r y c l o c k s o u r c e .
    uint32_t refId; // 32 b i t s . R e f e r e n c e c l o c k i d e n t i f i e r .
    uint32_t refTm_s; // 32 b i t s . R e f e r e n c e t i m e −stamp s e c o n d s .
    uint32_t refTm_f; // 32 b i t s . R e f e r e n c e t i m e −stamp f r a c t i o n o f a s e c o n d .
    uint32_t origTm_s; // 32 b i t s . O r i g i n a t e t i m e −stamp s e c o n d s .
    uint32_t origTm_f; // 32 b i t s . O r i g i n a t e t i m e −stamp f r a c t i o n o f a s e c o n d .
    uint32_t rxTm_s; // 32 b i t s . R e c e i v e d t i m e −stamp s e c o n d s .
    uint32_t rxTm_f; // 32 b i t s . R e c e i v e d t i m e −stamp f r a c t i o n o f a s e c o n d .
    uint32_t txTm_s ; // 32 b i t s and t h e most i m p o r t a n t f i e l d t h e c l i e n t c a r e s a b o u t . T r a n s m i t t i m e −stamps e c o n d s .
    uint32_t txTm_f; // 32 b i t s . T r a n s m i t t i m e −stamp f r a c t i o n o f a s e c o n d .
} ntp_packet; // T o t a l : 384 b i t s o r 48 b y t e s

void error(char *msg){
    perror(msg);
    _exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    ntp_packet packet;
    bzero(&packet, 48);

    SET_LI(packet, 0);
    SET_VN(packet, 3);
    SET_MODE(packet, 3);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        error("Socket error");
    }
    
    char host_name[] = "fr.pool.ntp.org";
    struct hostent *server;
    server = gethostbyname(host_name);

    _exit(EXIT_SUCCESS);
}