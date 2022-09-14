struct statistics
{
int num_videos_connections;

double time_of_all_videos;
double Average_duration_of_the_videos;

size_t sum_data_videos;
int Average_size_of_the_videos;

int sum_tran_videos;
int Average_number_of_TDRs_per_video;

size_t sum_data_transaction;
int Average_size_of_the_TDRs_per_video; /*data or size*/

double time_duration_transaction;
int Average_duration_of_the_TDRs_per_video;

double time_all_transaction;
int Average_time_between_two_consecutive_TDRs_in_a_video_connection;
};
void intiStatisticsStract(struct statistics *statis);
void writeCSVFile(struct statistics *statis);

