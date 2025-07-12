module libP2PServer //模块名（go.mod中的module声明）不需要和目录名一样。

require FmtLog v0.0.0

replace FmtLog => ./FmtLog

require DB v0.0.0

require (
	github.com/cespare/xxhash/v2 v2.3.0 // indirect
	github.com/dgryski/go-rendezvous v0.0.0-20200823014737-9f7001d12a5f // indirect
	github.com/redis/go-redis/v9 v9.11.0 // indirect
)

replace DB => ./DataBase

go 1.19
