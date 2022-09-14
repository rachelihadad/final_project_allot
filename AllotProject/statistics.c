#include <stdio.h>
#include "statistics.h"

struct statistics *statistic;
FILE *statisticsFile;
void intiStatisticsStract(struct statistics *statis)
{
    statis->sum_data_transaction = 0;
    statis->time_duration_transaction = 0;
    statis->time_all_transaction = 0;
    statis->sum_data_videos = 0;
    statis->sum_data_videos = 0;
    statis->num_videos_connections = 0;
    statis->time_of_all_videos = 0;
    statis->Average_duration_of_the_videos = 0;
    statis->Average_size_of_the_videos = 0;
    statis->Average_number_of_TDRs_per_video = 0;
    statis->Average_size_of_the_TDRs_per_video = 0;
    statis->Average_duration_of_the_TDRs_per_video = 0;
    statis->Average_time_between_two_consecutive_TDRs_in_a_video_connection = 0;
}

double time_duration_transaction;
int Average_duration_of_the_TDRs_per_video;

double time_all_transaction;
int Average_time_between_two_consecutive_TDRs_in_a_video_connection;
void writeCSVFile(struct statistics *statis)
{
    statisticsFile = fopen("statistics.csv", "w");
    if (statisticsFile == NULL)
    {
        printf("Unable to create statistics.csv file.");
    }
    int num_conn = statis->num_videos_connections;
    int num_trun = statis->sum_tran_videos;
    fprintf(statisticsFile, "%s ,%d\n", "num_videos_connections:  ", num_conn);
    fprintf(statisticsFile, "%s, %d\n", "num_tran_videos:  ", num_trun);
    fprintf(statisticsFile, "%s, %f\n", "Average_duration_of_the_videos:  ", statis->time_of_all_videos / num_conn);
    fprintf(statisticsFile, "%s, %ld\n","Average_size_of_the_videos: (data)", statis->sum_data_videos / num_conn);
    fprintf(statisticsFile, "%s, %d\n", "Average_number_of_TDRs_per_video: ", statis->sum_tran_videos / num_conn);
    fprintf(statisticsFile, "%s, %ld\n", "Average_size_of_the_TDRs_per_video: (data) ", statis->sum_data_videos / num_trun);
    fprintf(statisticsFile, "%s, %f\n", "Average_duration_of_the_TDRs_per_video: ", statis->time_duration_transaction / num_trun);
    fprintf(statisticsFile, "%s, %f\n", "Average_time_between_two_consecutive_TDRs_in_a_video_connection:", statis->time_all_transaction / num_trun);
    fclose(statisticsFile);
}
