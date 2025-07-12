module P2PServerDemo

//如果使用main.zip解压出来的main文件夹，则要在main文件夹下执行go build main.go，同时要删除掉项目根目录的go.mod main.go文件，以及删除掉src/NetIO.go

require libP2PServer v0.0.0

replace libP2PServer => ./src //这个要目录名src

require FmtLog v0.0.0

replace FmtLog => ./src/FmtLog

require DB v0.0.0 // indirect //内部子模块依赖的模块，外面这里也要声明，否则会报错

require (
	github.com/cespare/xxhash/v2 v2.3.0 // indirect
	github.com/dgryski/go-rendezvous v0.0.0-20200823014737-9f7001d12a5f // indirect
	github.com/redis/go-redis/v9 v9.11.0 // indirect
)

replace DB => ./src/DataBase

go 1.19
