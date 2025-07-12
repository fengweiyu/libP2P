package libP2PServer //包名不需要和目录名相同，但在同一目录的文件必须声明同一包名

//在Go中，同一目录下的所有.go文件必须声明相同的包名（package关键字后的名字）。
//具体原因：Go代码编译时会将同一目录的文件视作一个包（package），这些文件必须声明相同的包名，才能在编译时组合成一个整体。
//如果不同文件声明不同的包名，编译器会报错，提示包名不一致。

//包内所有文件属于同一包，因此它们在逻辑上是“共享”的，可以直接调用彼此的未导出和导出的标识符（函数、方法、类型、字段）。
//导出（exported）：首字母大写的标识符可以在包外部被调用；首字母小写的私有（unexported）标识符只能在包内部使用。

import (
	"fmt"
	"time"
)

// 常量定义
const (
	LOGIN_MSG_ID                    = 6701
	REPORT_NAT_INFO_MSG_ID          = 6702
	GET_NAT_INFO_MSG_ID             = 6703
	GET_NAT_INFO_ACK_MSG_ID         = 6704
	REQ_PEER_SEND_MSG_MSG_ID        = 6705
	REQ_PEER_SEND_MSG_ACK_MSG_ID    = 6706
	REQ_SEND_MSG_TO_PEER_MSG_ID     = 6707
	REQ_SEND_MSG_TO_PEER_ACK_MSG_ID = 6708
	REPORT_P2P_RESULT_MSG_ID        = 6709
	REPORT_P2P_RESULT_ACK_MSG_ID    = 6710
)

// Peer2PeerCfg 对等到对等配置结构体
type Peer2PeerCfg struct {
	StunServer1Addr string `json:"StunServer1Addr"`
	StunServer1Port int    `json:"StunServer1Port"`
	StunServer2Addr string `json:"StunServer2Addr"`
	StunServer2Port int    `json:"StunServer2Port"`
}

// NatInfoMsg NAT信息结构体
type NatInfoMsg struct {
	ID         string `json:"ID"`
	NatType    int    `json:"NatType"`
	PublicIP   string `json:"PublicIP"`
	PublicPort int    `json:"PublicPort"`
}

// ReqPeerSendMsg 请求对等发送消息结构体
type ReqPeerSendMsg struct {
	PeerID       string     `json:"PeerID"`
	LocalNatInfo NatInfoMsg `json:"LocalNatInfo"`
}

// ReqPeerSendAckMsg 请求对等发送确认消息结构体
type ReqPeerSendAckMsg struct {
	Result      int        `json:"Result"`
	PeerNatInfo NatInfoMsg `json:"PeerNatInfo"`
}

// ReqSendMsgToPeerResultMsg 请求发送消息到对等结果消息结构体
type ReqSendMsgToPeerResultMsg struct {
	LocalID string `json:"LocalID"`
	PeerID  string `json:"PeerID"`
	Result  int    `json:"Result"`
}

// PeerToPeerResultMsg 对等到对等结果消息结构体
type PeerToPeerResultMsg struct {
	LocalID    string `json:"LocalID"`
	PeerID     string `json:"PeerID"`
	SuccessCnt int    `json:"SuccessCnt"`
	FailCnt    int    `json:"FailCnt"`
	CurStatus  int    `json:"CurStatus"` // -1 失败, 0 成功
}

// QueueMessage 队列消息结构体
type QueueMessage struct {
	MsgID    int
	Data     []byte
	DataSize int
	Sender   interface{}
}

// ThreadSafeQueue 线程安全队列接口 定义一个包含缓冲通道的结构体 使用通道实现队列
type ThreadMsgQueue struct {
	ch chan QueueMessage //Go的通道（channel）是设计为线程安全的，可以多个goroutine同时用来传递数据，保证不会出现竞态条件。
}

// NewThreadMsgQueue 创建一个新的线程安全队列
func NewThreadMsgQueue() *ThreadMsgQueue {
	// 这里应该返回一个实现了ThreadSafeQueue接口的具体类型
	return &ThreadMsgQueue{
		ch: make(chan QueueMessage, 100), //最大个数100
	}
}

// (mq *ThreadMsgQueue) 类似类名，表示方法属于ThreadMsgQueue类
// 等待以及Pop从队列中弹出消息 ,下面的select语句会会一直等待到两个事件中的一个到达
func (mq *ThreadMsgQueue) WaitAndPop(msg *QueueMessage, timeoutMS int) int {
	var ret int = -1
	select {
	case msgR, ok := <-mq.ch:
		if !ok {
			fmt.Println("通道已关闭，退出接收") //从已关闭的通道接收 如果有剩余数据，接收剩余数据，否则立即返回零值
		}
		*msg = msgR
		ret = 0
	case <-time.After(time.Duration(timeoutMS) * time.Millisecond): //如果缓冲区为空（没有消息在队列中）value := <-ch将阻塞，直到有消息被发送到通道
		// 超时处理，如果在250毫秒内没有消息，触发
		//fmt.Println("超时，没有收到新消息")
	}
	return ret
}

// Push将消息推入队列 ,select会等待case中有能够执行的case时去执行 如果多个case同时就绪时，select会随机地选择一个执行 每次select只会处理满足条件的一个case，然后退出select语句块
func (mq *ThreadMsgQueue) Push(msg QueueMessage) int {
	var ret int = -1
	select {
	case mq.ch <- msg: //向已关闭的通道发送 运行时 panic（程序崩溃）
		ret = 0
	default: //select会有一个default来设置当其它的操作都不能够马上被处理时程序需要执行哪些逻辑
		fmt.Printf("发送失败（缓冲满），%d:%d", cap(mq.ch), len(mq.ch)) //如果缓冲区已满 继续执行ch <- value时会阻塞
	} //即使通道已关闭，len(ch) 仍然会返回通道中剩余的元素个数 //不受通道是否关闭的影响，cap始终返回通道的容量大小（构造时定义的缓冲区大小）
	return ret
}

// 关闭队列
func (mq *ThreadMsgQueue) Close() {
	close(mq.ch)
}

/* 使用条件变量无法做到超时等待，改为通道实现
// ThreadSafeQueue 线程安全队列接口
type ThreadMsgQueue struct {
	//Push(msg QueueMessage) int//函数要放interface里，interface里不能放函数
	queue  []QueueMessage
	lock   sync.Mutex // 保护队列
	cond   *sync.Cond // 条件变量，用于唤醒等待者
	closed bool       // 队列是否已关闭
}

// NewThreadMsgQueue 创建一个新的线程安全队列
func NewThreadMsgQueue() *ThreadMsgQueue {
	// 这里应该返回一个实现了ThreadSafeQueue接口的具体类型
	mq := &ThreadMsgQueue{
		queue: make([]QueueMessage, 0),
	}
	mq.cond = sync.NewCond(&mq.lock)
	mq.closed = false
	return mq
}
// 消费（阻塞等待直到有消息或队列关闭）
func (mq *ThreadMsgQueue) WaitAndPop(msg *QueueMessage) int {
	mq.lock.Lock()
	defer mq.lock.Unlock()

	for len(mq.queue) == 0 && !mq.closed {
		mq.cond.Wait() // 阻塞等待通知  只能阻塞不能带超时时间
	}
	if len(mq.queue) == 0 && mq.closed {
		// 队列已关闭且没有消息
		return -1
	}
	// 取出第一个消息
	*msg = mq.queue[0]
	mq.queue = mq.queue[1:]
	return 0
}

// Push 将消息推入队列
func (mq *ThreadMsgQueue) Push(msg QueueMessage) int {
	mq.lock.Lock()
	defer mq.lock.Unlock()

	if mq.closed {
		return -1
	}

	mq.queue = append(mq.queue, msg)
	mq.cond.Signal() // 唤醒等待的消费者
	return 0
}

// Pop 从队列中弹出消息 下面的select语句会会一直等待到两个事件中的一个到达
func (mq *ThreadMsgQueue) Pop(msg *QueueMessage) int {
	if mq.closed {
		return -1
	}
	if len(mq.queue) == 0 {
		return -1
	}
	*msg = mq.queue[0]
	mq.queue = mq.queue[1:]
	return 0
}

// IsEmpty 检查队列是否为空
func (mq *ThreadMsgQueue) IsEmpty() bool {
	return len(mq.queue) == 0
}

// 关闭队列
func (mq *ThreadMsgQueue) Close() {
	mq.lock.Lock()
	defer mq.lock.Unlock()
	mq.closed = true
	mq.cond.Broadcast() // 通知所有等待的消费者
}
*/
