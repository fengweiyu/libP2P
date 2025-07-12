package main //main包特殊，只能包含一个文件，即只能这一个文件声明为main包，其他文件不能声明//在Go中，同一目录下只能有一个文件声明为package main，且这个package main的文件必须包含main()函数，作为程序的入口点
//在Go中，包含main()函数的文件必须声明为package main。
//具体原因：main()函数是Go应用程序的入口点，只有在package main中才能被识别为“可执行程序”的入口。

import (
	"FmtLog"
	"fmt"
	"libP2PServer"
	"net"
)

// 常量定义
const (
	SERVER_LISTEN_PORT = 9128
)

func P2PMgr(mgrQueue *libP2PServer.ThreadMsgQueue) {
	oPeer2PeerManager := libP2PServer.NewPeer2PeerManager()
	for {
		oPeer2PeerManager.Proc(mgrQueue)
	}
}
func main() {
	mgrQueue := libP2PServer.NewThreadMsgQueue() //在新变量声明时自动推断类型，并初始化
	var cfg libP2PServer.Peer2PeerCfg
	cfg.StunServer1Addr = "stun.voipbuster.com"
	cfg.StunServer1Port = 3478
	cfg.StunServer2Addr = "gwm-000-cn-0448.bcloud365.net"
	cfg.StunServer2Port = 3478

	// 监听TCP端口
	str := fmt.Sprintf(":%d", SERVER_LISTEN_PORT)
	listener, err := net.Listen("tcp", str)
	if err != nil {
		FmtLog.ERR("监听失败: %v\n", err)
		return
	}
	defer listener.Close() // 确保监听器最终关闭

	FmtLog.INFO("TCP服务器已启动,监听端口 %d\r\n", SERVER_LISTEN_PORT)

	go P2PMgr(mgrQueue)

	for {
		// 等待客户端连接
		conn, err := listener.Accept()
		if err != nil {
			FmtLog.ERR("接受连接失败: %v\n", err)
			continue
		}

		// 为每个新连接启动一个goroutine
		go libP2PServer.Proc(mgrQueue, &cfg, conn)
	}
}
