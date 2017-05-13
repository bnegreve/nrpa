#!/bin/bash

awk 'BEGIN  { xmax = 0; ymax = 0; }                                                     \
     /^[^#]/{ count[$2]++; xsum[$2]+=$3; sum[$2]+=$4; sqsum[$2]+=$4*$4;                 \
              if($3 > xmax) { xmax=$3; };                                               \
              if($4 > ymax) { ymax=$4; };                                               \
     }                                                                                  \
     END    {                                                                    	\
              print 0, 0, 0, 0, 0, 0, 0;                                                \
              for( i in count){					                        \
                xavg=xsum[i] / count[i];                                                \
                yavg=sum[i] / count[i]; 						\
                ystdev=sqrt( (1/count[i]) * (sqsum[i] - (sum[i]*sum[i]/count[i]))); 	\
                yci= 1.96 * (ystdev / sqrt(count[i])); 					\
          	print i,								\
          	    xavg,								\
          	    yavg,								\
          	    ystdev,        							\
          	    xmax,          							\
          	    ymax,                                                               \
                    yci                                                                 \
       	     }                                                                          \
     }' $*
