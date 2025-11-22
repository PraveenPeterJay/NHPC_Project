#include "ring_seg_allreduce.h"
#include <string.h>
#include <stdlib.h>

void ring_seg_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    double *send_buf = (double*)sendbuf;
    double *recv_buf = (double*)recvbuf;
    
    memcpy(recv_buf, send_buf, count * sizeof(double));
    
    // Segment size (default 8KB = 1024 doubles)
    ll segment_size = 1024;
    if (segment_size > count / size) segment_size = count / size;
    
    ll chunk_size = count / size;
    ll num_segments = (chunk_size + segment_size - 1) / segment_size;
    
    double *temp_buf = (double*)malloc(segment_size * sizeof(double));
    
    int left = (rank - 1 + size) % size;
    int right = (rank + 1) % size;
    
    // PHASE 1: REDUCE-SCATTER with segmentation
    for (int step = 0; step < size - 1; step++) {
        int send_chunk = (rank - step + size) % size;
        int recv_chunk = (rank - step - 1 + size) % size;
        
        ll chunk_offset_send = send_chunk * chunk_size;
        ll chunk_offset_recv = recv_chunk * chunk_size;
        
        for (ll seg = 0; seg < num_segments; seg++) {
            ll seg_start = seg * segment_size;
            ll seg_count = segment_size;
            if (seg_start + seg_count > chunk_size) {
                seg_count = chunk_size - seg_start;
            }
            
            MPI_Sendrecv(&recv_buf[chunk_offset_send + seg_start], seg_count, 
                         MPI_DOUBLE, right, seg,
                         temp_buf, seg_count, MPI_DOUBLE, left, seg,
                         comm, MPI_STATUS_IGNORE);
            
            for (ll i = 0; i < seg_count; i++) {
                recv_buf[chunk_offset_recv + seg_start + i] += temp_buf[i];
            }
        }
    }
    
    // PHASE 2: ALLGATHER with segmentation
    for (int step = 0; step < size - 1; step++) {
        int send_chunk = (rank - step - 1 + size) % size;
        int recv_chunk = (rank - step - 2 + size) % size;
        
        ll chunk_offset_send = send_chunk * chunk_size;
        ll chunk_offset_recv = recv_chunk * chunk_size;
        
        for (ll seg = 0; seg < num_segments; seg++) {
            ll seg_start = seg * segment_size;
            ll seg_count = segment_size;
            if (seg_start + seg_count > chunk_size) {
                seg_count = chunk_size - seg_start;
            }
            
            MPI_Sendrecv(&recv_buf[chunk_offset_send + seg_start], seg_count,
                         MPI_DOUBLE, right, seg,
                         &recv_buf[chunk_offset_recv + seg_start], seg_count,
                         MPI_DOUBLE, left, seg,
                         comm, MPI_STATUS_IGNORE);
        }
    }
    
    free(temp_buf);
}