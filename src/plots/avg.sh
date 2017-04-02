#!/bin/bash

awk '/^[^#]/{ count[$2]++; xsum[$2]+=$3; sum[$2]+=$4; sqsum[$2]+=$4*$4 }        \
END{                                                                    	\
    for(i =0; i < length(count); i++){						\
	print i,								\
	    xsum[i] / count[i],							\
	    sum[i] / count[i],							\
	    sqrt( (1/count[i]) * (sqsum[i] - (sum[i]*sum[i]/count[i])))         \
	    }                                                                   \
}' $1
