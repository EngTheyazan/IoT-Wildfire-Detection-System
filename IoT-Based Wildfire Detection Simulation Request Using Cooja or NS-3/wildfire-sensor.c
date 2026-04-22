#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"

#define LOG_MODULE "WildfireSensor"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (30 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;

PROCESS(wildfire_sensor_process, "Wildfire Sensor Process");
AUTOSTART_PROCESSES(&wildfire_sensor_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}

PROCESS_THREAD(wildfire_sensor_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, 8080, NULL, 8080, udp_rx_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      char buf[32];
      snprintf(buf, sizeof(buf), "hello %u", count++);
      LOG_INFO("Sending request %s to ", buf);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      simple_udp_sendto(&udp_conn, buf, strlen(buf), &dest_ipaddr);
    }

    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}

