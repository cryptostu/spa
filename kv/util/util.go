package util

import "time"

func Microsecond() int64 {
	return time.Now().UnixNano() / 1e3
}
