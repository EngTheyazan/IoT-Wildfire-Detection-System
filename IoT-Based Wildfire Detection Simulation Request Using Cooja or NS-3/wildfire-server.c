#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"

#define LOG_MODULE "WildfireServer"
#define LOG_LEVEL LOG_LEVEL_INFO

static struct simple_udp_connection udp_conn;

PROCESS(wildfire_server_process, "Wildfire Server Process");
AUTOSTART_PROCESSES(&wildfire_server_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  simple_udp_sendto(c, data, datalen, sender_addr);
}

PROCESS_THREAD(wildfire_server_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();

  simple_udp_register(&udp_conn, 8080, NULL, 8080, udp_rx_callback);

  PROCESS_END();
}

