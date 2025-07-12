package libP2PServer //包名不需要和目录名相同，但在同一目录的文件必须声明同一包名//net要和目录名一致，不然其他地方引用报错

import (
	"FmtLog"
	"io"
	"net"
	"time"
)

func Proc(mgrQueue *ThreadMsgQueue, cfg *Peer2PeerCfg, oSession net.Conn) {
	defer oSession.Close() // 确保连接最终关闭

	oServerSession := NewServerSession(mgrQueue, cfg) //在新变量声明时自动推断类型，并初始化

	FmtLog.INFO("新连接来自: %s\n", oSession.RemoteAddr().String())
	// 设置超时时间，例如5秒
	timeoutDuration := 500 * time.Millisecond

	//reader := bufio.NewReader(oSession)  // 创建一个带缓冲的读取器
	buf := make([]byte, 1024) //对象超出作用域或对象没有被引用(不可达)自动释放，例如提前释放使用buf=nil，放外面省的重复申请内存
	bufRes := make([]byte, 1024)
	for {
		// 设置读超时
		err := oSession.SetReadDeadline(time.Now().Add(timeoutDuration))
		if err != nil {
			FmtLog.ERR("设置超时失败:%v\n", err)
			return
		}

		n, err := oSession.Read(buf)

		if err != nil {
			if netErr, ok := err.(net.Error); ok && netErr.Timeout() { // 超时没有数据
				ret := oServerSession.Proc(string(buf[:n]), n, bufRes, 1024)
				if ret > 0 {
					oSession.Write(bufRes[:ret])
				}
				continue // 可以在这里做一些逻辑处理，比如继续等待、退出、保存状态等
			} else if err == io.EOF {
				FmtLog.WARN("连接被关闭")
				break
			} else {
				FmtLog.ERR("读取错误:%v\n", err)
				break
			}
		} else if n > 0 {
			FmtLog.INFO("收到数据 %s : %s\r\n", oSession.RemoteAddr().String(), string(buf[:n])) // 继续等待或处理
			ret := oServerSession.Proc(string(buf[:n]), n, bufRes, 1024)
			if ret > 0 {
				oSession.Write(bufRes[:ret])
			}
		}
	}

}
