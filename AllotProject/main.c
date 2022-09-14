#include <stdio.h>
#include <stdlib.h>
#include "pcap.h"
#include <stddef.h>
#include "packet_structs.h"
#include "structs.h"
#include "hashtable.h"
#include "list.h"
#include "pthread.h"
#include "statistics.h"
#include "json-c/json.h"
#include <time.h>
#include <math.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#define SIZE_ADRRESS 20
#define MIL_SEC 1000000.00000000
/*global variable*/
struct my_object *myobj;
struct connection *connections;
int random_conn = 1;
ht *hash;
struct packet *packet_a;
const char *key;
const char *temp_key;
struct emptyQueue *queue;
// pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER; // mutex for queue
// pthread_mutex_t connMutex = PTHREAD_MUTEX_INITIALIZER;  // mutex for connaction
FILE *CSVfile;
FILE *CSVfile_test;
struct timer *timer;
struct statistics *statistic;
char *cli_ip;
char *srv_ip;
struct in_addr *addr_cli;
struct in_addr *addr_srv;
struct list *emptyList;
struct list *liveList;
double time_diff_print(struct timeval x, struct timeval y)
{
  double x_ms, y_ms, diff;

  x_ms = (double)x.tv_sec - (double)y.tv_sec;
  y_ms = (double)x.tv_usec - (double)y.tv_usec;

  diff = (double)x_ms + (double)y_ms / MIL_SEC;

  return diff;
}
double time_diff(struct timeval y, struct timeval x)
{
  double x_ms, y_ms, diff;

  x_ms = (double)x.tv_sec + (double)x.tv_usec / MIL_SEC;
  y_ms = (double)y.tv_sec + (double)y.tv_usec / MIL_SEC;

  diff = (double)y_ms - (double)x_ms;

  return diff;
}
// void initQueue()
// {
//   queue->index = 0;
//   for (size_t i = 0; i < myobj->max_number_of_connections; i++)
//   {
//     queue->emptyQueueConn[i] = i + 1;
//   }
// }

// void insertEmptyConn(int emptyIndex)
// {
//   pthread_mutex_lock(&queueMutex);
//   if (queue->index == 0 || emptyIndex == 0)
//   {
//     pthread_mutex_unlock(&queueMutex);
//     return;
//   }
//   queue->emptyQueueConn[queue->index] = emptyIndex;
//   queue->index--;
//   pthread_mutex_unlock(&queueMutex);
// }

// int pullEmptyConn()
// {
//   int temp;
//   pthread_mutex_lock(&queueMutex);
//   temp = queue->emptyQueueConn[queue->index];
//   if (queue->index == myobj->max_number_of_connections || temp == 0)
//   {
//     printf("full: - %d - \n", myobj->max_number_of_connections);
//     pthread_mutex_unlock(&queueMutex);
//     return 0;
//   }
//   queue->index++;
//   pthread_mutex_unlock(&queueMutex);
//   return temp;
// }
void writeToCSVfile(int conn_id)
{
  statistic->num_videos_connections++;
  statistic->time_of_all_videos += time_diff(connections[conn_id].end, connections[conn_id].start);
  statistic->sum_data_videos += connections[conn_id].total_bandwidth;
  statistic->sum_tran_videos += connections[conn_id].current_tran + 1;
  struct transaction *all_trun = connections[conn_id].all_tran;
  struct Tuple5 *tuple = connections[conn_id].tuple;
  for (size_t i = 0; i < connections[conn_id].current_tran + 1; i++)
  {
    struct transaction trun = all_trun[i];
    if (i != connections[conn_id].current_tran)
      statistic->time_all_transaction += time_diff(all_trun[i + 1].start, all_trun[i].start);
    statistic->time_duration_transaction += time_diff(trun.time_live, trun.start);
    statistic->sum_data_transaction += trun.total_size_packet;
    double time_a = (double)trun.start.tv_sec + (double)trun.start.tv_usec / MIL_SEC;
    double rtt = 0;
    if (trun.flag_start)
      rtt = time_diff(trun.first_packet_from_the_server, trun.Time_request);
    if (trun.min_packet_size_inbound == INT32_MAX)
      trun.min_packet_size_inbound = 0;
    if (trun.min_diff_time_inbound == INT32_MAX)
      trun.min_diff_time_inbound = 0;
    addr_cli->s_addr = tuple->client_ip;
    addr_srv->s_addr = tuple->server_ip;
    sprintf(srv_ip, "%s", inet_ntoa(*addr_srv));
    sprintf(cli_ip, "%s", inet_ntoa(*addr_cli));
    fprintf(CSVfile, "%d,%d,%s,%u,%s,%u,%f,%f,%d,%d,%d,%d,%f,%f,%f\n",
            trun.transaction_id, trun.conn_id, srv_ip, tuple->server_port, cli_ip, tuple->client_port, time_a, rtt,
            trun.num_inbound_packets_in_range, trun.num_outbound_packets_in_range, trun.max_packet_size_inbound,
            trun.min_packet_size_inbound, trun.max_diff_time_inbound, trun.min_diff_time_inbound, trun.SumSquareInboundPacketTimeDiff);
  }
}
// void uptateConnaction(struct packet *packet_a, int conn_id)
// {
//   connections[conn_id].time_live.tv_sec = packet_a->time.tv_sec;
//   connections[conn_id].time_live.tv_usec = packet_a->time.tv_usec;
// }
void createNewTransaction(struct packet *packet_a, int conn_id)
{
  connections[conn_id].current_tran++;
  int current_tran = connections[conn_id].current_tran;
  // struct transaction tran = connections[conn_id].all_tran[current_tran];
  connections[conn_id].all_tran[current_tran].start.tv_sec = packet_a->time.tv_sec;
  connections[conn_id].all_tran[current_tran].start.tv_usec = packet_a->time.tv_usec;
  connections[conn_id].all_tran[current_tran].transaction_id = current_tran;
  connections[conn_id].all_tran[current_tran].conn_id = connections[conn_id].conn_id;
  connections[conn_id].all_tran[current_tran].total_size_packet = 0;
  connections[conn_id].all_tran[current_tran].flag_start = 0;
  connections[conn_id].all_tran[current_tran].num_inbound_packets_in_range = 0;
  connections[conn_id].all_tran[current_tran].num_outbound_packets_in_range = 0;
  connections[conn_id].all_tran[current_tran].min_packet_size_inbound = INT32_MAX;
  connections[conn_id].all_tran[current_tran].max_packet_size_inbound = 0;
  connections[conn_id].all_tran[current_tran].max_diff_time_inbound = 0;
  connections[conn_id].all_tran[current_tran].min_diff_time_inbound = INT32_MAX;
  connections[conn_id].all_tran[current_tran].Time_request.tv_sec = packet_a->time.tv_sec;
  connections[conn_id].all_tran[current_tran].Time_request.tv_usec = packet_a->time.tv_usec;
  connections[conn_id].all_tran[current_tran].time_last_packet.tv_sec = packet_a->time.tv_sec;
  connections[conn_id].all_tran[current_tran].time_last_packet.tv_sec = packet_a->time.tv_usec;
  connections[conn_id].all_tran[current_tran].time_live.tv_sec = packet_a->time.tv_sec;
  connections[conn_id].all_tran[current_tran].time_live.tv_usec = packet_a->time.tv_usec;
  connections[conn_id].all_tran[current_tran].SumInboundPacketTimeDiff = 0;
  connections[conn_id].all_tran[current_tran].SumSquareInboundPacketTimeDiff = 0;
  // connections[conn_id].all_tran[current_tran] = tran;
}

void closeTransaction(int conn_id)
{
}
struct Tuple5 *init5Tuple(struct packet *packet_a, int conn_id)
{
  struct Tuple5 *tuple = connections[conn_id].tuple;
  tuple->l4 = packet_a->version;
  if (packet_a->src_port == SERVER_PORT)
  {
    tuple->server_ip = packet_a->src_ip;
    tuple->server_port = packet_a->src_port;
    tuple->client_ip = packet_a->dest_ip;
    tuple->client_port = packet_a->dest_port;
  }
  if (packet_a->dest_port == SERVER_PORT)
  {
    tuple->server_ip = packet_a->dest_ip;
    tuple->server_port = packet_a->dest_port;
    tuple->client_ip = packet_a->src_ip;
    tuple->client_port = packet_a->src_port;
  }
  connections[conn_id].tuple = tuple;
}
int createNewConnaction(struct packet *packet_a, int conn_index)
{
  int indexConn;
  struct listelement *element;
  if (conn_index == -1) /* create new connaction */
  {
    element = pullFromList(emptyList);
    indexConn = element->val;
    insertToList(liveList, element);
    init5Tuple(packet_a, indexConn); /*init tuple*/
  }
  else /* override connaction - time out */
  {
    indexConn = conn_index;
    element = connections[indexConn].element;
    insertToFrontFromList(liveList, element);
  }
  struct connection conn = connections[indexConn];
  conn.element = element;
  conn.start_flag = 0;
  conn.status = 1;
  conn.indexConn = indexConn;
  conn.conn_id = random_conn; /*id*/
  random_conn++;
  conn.current_tran = -1;
  conn.total_bandwidth = 0;
  conn.time_live.tv_sec = packet_a->time.tv_sec;
  conn.time_live.tv_usec = packet_a->time.tv_usec;
  conn.start.tv_sec = packet_a->time.tv_sec;
  conn.start.tv_usec = packet_a->time.tv_usec;
  connections[indexConn] = conn;
  return conn.indexConn;
}
// int createNewConnactionFromConn(struct packet *packet_a, int conn_index)
// {
//   int indexConn = conn_index;
//   struct connection conn = connections[indexConn];
//   conn.indexConn = indexConn;
//   conn.conn_id = random_conn++; /*id*/
//   conn.current_tran = 0;
//   conn.total_bandwidth = 0;
//   conn.tuple = init5Tuple(packet_a, conn.conn_id); /*init tuple*/
//   uptateConnaction(packet_a, conn_index);
//   conn.time_live.tv_sec = packet_a->time.tv_sec;
//   conn.time_live.tv_usec = packet_a->time.tv_usec;
//   conn.status = 1;
//   conn.start = packet_a->time;
//   connections[indexConn] = conn;
//   return conn.indexConn;
// }
void closeConnaction(int id_conn, int flag)
{
  connections[id_conn].status = 0;
  connections[id_conn].end.tv_sec = connections[id_conn].time_live.tv_sec;
  connections[id_conn].end.tv_usec = connections[id_conn].time_live.tv_usec;
  if (connections[id_conn].total_bandwidth > myobj->minimum_video_connection_size && connections[id_conn].start_flag == 1)
    writeToCSVfile(id_conn);
  if (flag) /* close connaction and delete from hash and live_list and insert to empty list*/
  {
    struct listelement *element = pullFromList(liveList);
    if (element == NULL || id_conn != element->val)
      return;
    insertToList(emptyList, element);
    sprintf(temp_key, "%u%u%hu", connections[id_conn].tuple->server_ip, connections[id_conn].tuple->client_ip, connections[id_conn].tuple->client_port);
    ht_reset_element(hash, temp_key);
  }
}
// void closeConnactionFromConn(int id_conn)
// {
//   closeTransaction(id_conn);
//   // printf("close aaaaaaaa: - %d - \n", id_conn);
//   printf("num inbound : - %d  conn %d- \n", connections[id_conn].all_tran[connections[id_conn].current_tran].num_inbound_packets_in_range, connections[id_conn].conn_id);
//   if (connections[id_conn].total_bandwidth > myobj->minimum_video_connection_size)
//     writeToCSVfile(id_conn);
//   else
//     printf("not write from: - %d - \n", connections[id_conn].conn_id);
// }
// Inbound – כיוון מהשרת ללקוח
void fromServerToClient(struct packet *packet_a, int conn_id)
{
  if (connections[conn_id].start_flag == 0)
    return;
  int current_tran = connections[conn_id].current_tran;
  struct transaction tran = connections[conn_id].all_tran[current_tran];
  if (tran.flag_start == 0)
  {
    tran.time_last_packet.tv_sec = packet_a->time.tv_sec;
    tran.time_last_packet.tv_usec = packet_a->time.tv_usec;
  }
  else
  {
    if (time_diff(packet_a->time, tran.time_last_packet) < tran.min_diff_time_inbound)
      tran.min_diff_time_inbound = time_diff(packet_a->time, tran.time_last_packet);
  }
  if (packet_a->size_packet > myobj->inbound_packets_in_range_min && packet_a->size_packet < myobj->inbound_packets_in_range_max)
  {
    if (tran.flag_start == 0)
    {
      tran.first_packet_from_the_server.tv_sec = packet_a->time.tv_sec;
      tran.first_packet_from_the_server.tv_usec = packet_a->time.tv_usec;
      tran.flag_start = 1;
    }
    connections[conn_id].total_bandwidth += packet_a->size_data;
    tran.total_size_packet += packet_a->size_data;
    tran.num_inbound_packets_in_range += 1;
  }
  if (time_diff(packet_a->time, tran.time_last_packet) > tran.max_diff_time_inbound)
    tran.max_diff_time_inbound = time_diff(packet_a->time, tran.time_last_packet);

  double diff = time_diff(packet_a->time, tran.time_last_packet);
  tran.SumSquareInboundPacketTimeDiff += diff * diff;
  tran.time_live.tv_sec = packet_a->time.tv_sec;
  tran.time_live.tv_usec = packet_a->time.tv_usec;
  tran.time_last_packet.tv_sec = packet_a->time.tv_sec;
  tran.time_last_packet.tv_usec = packet_a->time.tv_usec;
  if (packet_a->size_packet > tran.max_packet_size_inbound)
    tran.max_packet_size_inbound = packet_a->size_packet;
  if (packet_a->size_packet < tran.min_packet_size_inbound)
    tran.min_packet_size_inbound = packet_a->size_packet;

  connections[conn_id].all_tran[current_tran] = tran;
}
void fromClientToServer(struct packet *packet_a, int conn_id)
{
  if (packet_a->size_packet >= myobj->request_packet_threshold) // len packet it is in range
  {
    connections[conn_id].start_flag = 1;
    closeTransaction(conn_id);
    createNewTransaction(packet_a, conn_id);
  }
  else
  {
    if (connections[conn_id].start_flag == 0)
      return;
    connections[conn_id].all_tran[connections[conn_id].current_tran].num_outbound_packets_in_range += 1;
    connections[conn_id].all_tran[connections[conn_id].current_tran].time_live.tv_sec = packet_a->time.tv_sec;
    connections[conn_id].all_tran[connections[conn_id].current_tran].time_live.tv_usec = packet_a->time.tv_usec;
  }
}

void initPacket(const struct pcap_pkthdr *header, const unsigned char *packet)
{
  const struct sniff_ip *ip = (struct sniff_ip *)(packet + SIZE_ETHERNET);
  int size_ip = IP_HL(ip) * 4;
  struct sniff_udp *udp = (struct sniff_udp *)(packet + SIZE_ETHERNET + size_ip);
  int total_headers_size = SIZE_ETHERNET + size_ip + UDPHDRLEN;

  int payload_length = header->caplen - total_headers_size;
  packet_a->src_ip = ip->ip_src.s_addr;
  packet_a->dest_ip = ip->ip_dst.s_addr;
  packet_a->src_port = ntohs(udp->udp_srcport);
  packet_a->dest_port = ntohs(udp->udp_destport);
  packet_a->time.tv_sec = header->ts.tv_sec;
  packet_a->time.tv_usec = header->ts.tv_usec;
  packet_a->size_packet = header->len;
  packet_a->size_data = payload_length;
  packet_a->version = ip->ip_vhl;
}
int filterPacket(const struct pcap_pkthdr *header, const unsigned char *packet)
{
  struct sniff_ethernet *ethernet = (struct sniff_ethernet *)(packet);
  const struct sniff_ip *ip = (struct sniff_ip *)(packet + SIZE_ETHERNET);
  int size_ip = IP_HL(ip) * 4;
  struct sniff_udp *udp = (struct sniff_udp *)(packet + SIZE_ETHERNET + size_ip);
  u_short eth_type = ntohs(ethernet->ether_type);
  struct ether_header *eth_header;
  eth_header = (struct ether_header *)packet;
  if (eth_type != ETHERTYPE_IP || ip->ip_p != IPPROTO_UDP || (ntohs(udp->udp_srcport) != SERVER_PORT && (ntohs(udp->udp_destport)) != SERVER_PORT))
    return 0;
  return 1;
}
void hashingKey()
{
  uint32_t client_ip;
  uint16_t client_port;
  uint32_t server_ip;
  uint16_t server_port;
  if (packet_a->src_port == SERVER_PORT)
  {
    server_ip = packet_a->src_ip;
    server_port = packet_a->src_port;
    client_ip = packet_a->dest_ip;
    client_port = packet_a->dest_port;
  }
  else
  {
    server_ip = packet_a->dest_ip;
    server_port = packet_a->dest_port;
    client_ip = packet_a->src_ip;
    client_port = packet_a->src_port;
  }

  sprintf(key, "%u%u%hu", server_ip, client_ip, client_port);
}
void update_timer()
{
  timer->lockal_time.tv_sec = packet_a->time.tv_sec;
  timer->lockal_time.tv_usec = packet_a->time.tv_usec;
}
// void closeConnections()
// {
//   for (size_t i = 0; i < myobj->max_number_of_connections; i++)
//   {
//     if (connections[i].status == 0)
//       continue;
//     if (difftime(timer->tv_sec, connections[i].time_live.tv_sec) > 20) // if time out to connaction
//       closeConnaction(i, 1);
//   }
// }
packet_handler(struct packet *packet_a, int conn_index)
{
  if (connections[conn_index].status == 1)
  {
    if (difftime(packet_a->time.tv_sec, connections[conn_index].time_live.tv_sec) > myobj->video_connection_timeout) // if time out to connaction
    {
      closeConnaction(conn_index, 0);
      createNewConnaction(packet_a, conn_index);
    }
  }

  connections[conn_index].time_live.tv_sec = packet_a->time.tv_sec;
  connections[conn_index].time_live.tv_usec = packet_a->time.tv_usec;
  insertToFrontFromList(liveList, connections[conn_index].element); /* replace to head  live_list*/
  if (packet_a->src_port == SERVER_PORT)
    fromServerToClient(packet_a, conn_index);
  if (packet_a->dest_port == SERVER_PORT)
    fromClientToServer(packet_a, conn_index);
}
void closeConnactionAllTime()
{
  timer->lockal_time.tv_sec = packet_a->time.tv_sec;
  timer->lockal_time.tv_usec = packet_a->time.tv_usec;
  if (getEndElementList(liveList) == NULL)
    return;
  struct listelement *element = getEndElementList(liveList);
  int index = element->val;
  while (time_diff(timer->lockal_time, connections[index].time_live) > myobj->video_connection_timeout)
  {
    closeConnaction(index, 1);
    if (getEndElementList(liveList) == NULL)
      break;
    element = getEndElementList(liveList);
    index = element->val;
  }
  // if (time_diff(packet_a->time, connections[index].time_live) > myobj->video_connection_timeout)
  // {
  //   closeConnaction(index, 1);
  // }
}
void my_packet_handler(unsigned char *args, const struct pcap_pkthdr *header, const unsigned char *packet)
{
  if (!filterPacket(header, packet))
    return;
  initPacket(header, packet);
  int conn_index;
  hashingKey();
  //closeConnactionAllTime();
  if (ht_get(hash, key) != 0)
    conn_index = ht_get(hash, key);
  else
  {
    conn_index = createNewConnaction(packet_a, -1); // create a new connaction
    ht_set(hash, key, conn_index);                  // set hash conn_id
  }

  packet_handler(packet_a, conn_index);
}

void threadFunction()
{
  while (1)
  {
    closeConnactionAllTime();
  }
}
void initConnaction()
{
  connections = (struct connection *)malloc(sizeof(struct connection) * myobj->max_number_of_connections);
  for (size_t i = 0; i < myobj->max_number_of_connections; i++)
  {
    connections[i].all_tran = (struct transaction *)malloc(sizeof(struct transaction) * myobj->max_number_of_transaction_per_video);
    connections[i].tuple = malloc(sizeof(struct Tuple5));
    connections[i].status = 0;
  }
}
void ReadConfigFile()
{
  // for json file
  char buffer[PCAP_BUF_SIZE];
  FILE *fi;
  // open json file
  fi = fopen("init.json", "r");
  // read data from json file to buffer
  fread(buffer, PCAP_BUF_SIZE, 1, fi);
  // close json file
  fclose(fi);
  // parse data from buffer to json object
  struct json_object *parsed_j;
  parsed_j = json_tokener_parse(buffer);
  struct json_object *jrequest_packet_threshold;
  struct json_object *jminimum_video_connection_size;
  struct json_object *jinbound_packets_in_range_min;
  struct json_object *jinbound_packets_in_range_max;
  struct json_object *joutbound_packets_in_range_min;
  struct json_object *joutbound_packets_in_range_max;
  struct json_object *jmax_diff_time_inbound_threshold;
  struct json_object *jmin_diff_time_inbound_threshold;
  struct json_object *jnumber_of_videos_to_output_statistics_per_video;
  struct json_object *jmax_number_of_connections;
  struct json_object *jmax_number_of_transaction_per_video;
  struct json_object *jvideo_connection_timeout;
  json_object_object_get_ex(parsed_j, "request_packet_threshold", &jrequest_packet_threshold);
  json_object_object_get_ex(parsed_j, "minimum_video_connection_size", &jminimum_video_connection_size);
  json_object_object_get_ex(parsed_j, "inbound_packets_in_range_min", &jinbound_packets_in_range_min);
  json_object_object_get_ex(parsed_j, "inbound_packets_in_range_max", &jinbound_packets_in_range_max);
  json_object_object_get_ex(parsed_j, "outbound_packets_in_range_min", &joutbound_packets_in_range_min);
  json_object_object_get_ex(parsed_j, "outbound_packets_in_range_max", &joutbound_packets_in_range_max);
  json_object_object_get_ex(parsed_j, "max_diff_time_inbound_threshold", &jmax_diff_time_inbound_threshold);
  json_object_object_get_ex(parsed_j, "min_diff_time_inbound_threshold", &jmin_diff_time_inbound_threshold);
  json_object_object_get_ex(parsed_j, "number_of_videos_to_output_statistics_per_video", &jnumber_of_videos_to_output_statistics_per_video);
  json_object_object_get_ex(parsed_j, "max_number_of_connections", &jmax_number_of_connections);
  json_object_object_get_ex(parsed_j, "max_number_of_transaction_per_video", &jmax_number_of_transaction_per_video);
  json_object_object_get_ex(parsed_j, "video_connection_timeout", &jvideo_connection_timeout);
  myobj->request_packet_threshold = json_object_get_int(jrequest_packet_threshold);
  myobj->minimum_video_connection_size = json_object_get_int(jminimum_video_connection_size);
  myobj->inbound_packets_in_range_min = json_object_get_int(jinbound_packets_in_range_min);
  myobj->inbound_packets_in_range_max = json_object_get_int(jinbound_packets_in_range_max);
  myobj->outbound_packets_in_range_min = json_object_get_int(joutbound_packets_in_range_min);
  myobj->outbound_packets_in_range_max = json_object_get_int(joutbound_packets_in_range_max);
  myobj->max_diff_time_inbound_threshold = json_object_get_int(jmax_diff_time_inbound_threshold);
  myobj->min_diff_time_inbound_threshold = json_object_get_int(jmin_diff_time_inbound_threshold);
  myobj->number_of_videos_to_output_statistics_per_video = json_object_get_int(jnumber_of_videos_to_output_statistics_per_video);
  myobj->max_number_of_connections = json_object_get_int(jmax_number_of_connections);
  myobj->max_number_of_transaction_per_video = json_object_get_int(jmax_number_of_transaction_per_video);
  myobj->video_connection_timeout = json_object_get_int(jvideo_connection_timeout);
}

void closeAll()
{
  while (liveList->head != NULL)
  {
    struct listelement *element = getEndElementList(liveList);
    closeConnaction(element->val, 1);
  }
}
void freeAll()
{
  free((void *)statistic);
  free((void *)cli_ip);
  free((void *)srv_ip);
  free((void *)addr_srv);
  free((void *)key);
  free((void *)packet_a);
  free((void *)timer);
  free((void *)myobj);
  for (size_t i = 0; i < myobj->max_number_of_connections; i++)
  {
    free((void *)connections[i].tuple);
    free((void *)connections[i].all_tran);
  }
  free((void *)connections);
  struct listelement *tmp;
  struct listelement *head = emptyList->head;
  while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free((void *)tmp);
    }
}
int main(int argc, char **argv)
{
  char error_buffer[PCAP_ERRBUF_SIZE];
  char file[] = "capture_file.pcap";
  pcap_t *handle = pcap_open_offline(file, error_buffer);
  if (handle == NULL)
  {
    fprintf(stderr, "Couldn't open file %s: %s\n", file, error_buffer);
    exit(EXIT_FAILURE);
  }
  CSVfile = fopen("output.csv", "w");
  if (CSVfile == NULL)
  {
    printf("Unable to create output.csv file.");
  }
  fprintf(CSVfile, "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n",
          "transaction_id", "conn_id", "server_ip", "server_port", "client_ip",
          "client_port", "start time", "RTT", "num_inbound_packets", "num_outbound_packets",
          "max_packet_size_inbound", "min_packet_size_inbound", "max_diff_time_inbound",
          "min_diff_time_inbound", "SumSquareInboundPacketTimeDiff");
  myobj = (struct my_object *)malloc(sizeof(struct my_object));
  ReadConfigFile();
  hash = ht_create(); /* init hash */
  timer = malloc(sizeof(struct timer));
  packet_a = (struct packet *)malloc(sizeof(struct packet));
  key = (char *)malloc(sizeof(char) * SIZE_ADRRESS);
  temp_key = (char *)malloc(sizeof(char) * SIZE_ADRRESS);
  // queue = (struct emptyQueue *)malloc(sizeof(struct emptyQueue));
  // queue->emptyQueueConn = (int *)malloc(sizeof(int) * myobj->max_number_of_connections);
  addr_cli = malloc(sizeof(struct in_addr));
  addr_srv = malloc(sizeof(struct in_addr));
  srv_ip = malloc(SIZE_ADRRESS);
  cli_ip = malloc(SIZE_ADRRESS);
  statistic = malloc(sizeof(struct statistics));
  // initQueue();  /*init queue*/
  initConnaction();
  intiStatisticsStract(statistic);
  emptyList = malloc(sizeof(struct list));
  liveList = malloc(sizeof(struct list));
  liveList->head = NULL;
  initEmptyList(emptyList,myobj->max_number_of_connections);
  // pthread_t thread;
  // pthread_create(&thread, NULL, threadFunction, NULL);
  pcap_loop(handle, 0, my_packet_handler, NULL);
  closeAll();
  writeCSVFile(statistic);
  freeAll();
  fclose(CSVfile);
  ht_destroy(hash);
}