package libP2PServer //包名不需要和目录名相同，但在同一目录的文件必须声明同一包名

//在Go中，同一目录下的所有.go文件必须声明相同的包名（package关键字后的名字）。
//具体原因：Go代码编译时会将同一目录的文件视作一个包（package），这些文件必须声明相同的包名，才能在编译时组合成一个整体。
//如果不同文件声明不同的包名，编译器会报错，提示包名不一致。

//包内所有文件属于同一包，因此它们在逻辑上是“共享”的，可以直接调用彼此的未导出和导出的标识符（函数、方法、类型、字段）。
//导出（exported）：首字母大写的标识符可以在包外部被调用；首字母小写的私有（unexported）标识符只能在包内部使用。

import (
	"DB"
	"FmtLog" //库名和包名一样的情况，直接引用//如果一个库里有多个包，则要库/包的形式引用 ,一个项目（模块）通常由多个包组成//引用其他包必须前面带库(模块)名，如果库名和包名一样的情况，可以直接引用
	"encoding/json"
)

// NAT信息结构体
type NatInfo struct {
	pSession    interface{}
	iNatType    int
	iPublicPort int
	strPublicIP string
}

// P2P结果结构体
type Peer2PeerResult struct {
	strLocalID  string
	strPeerID   string
	iSuccessCnt int
	iFailCnt    int
	iCurStatus  int
}

// Peer2PeerManager 主结构体
type Peer2PeerManager struct {
	pMgrQueue          *ThreadMsgQueue
	NatInfoMap         map[string]NatInfo
	Peer2PeerResultMap map[string]Peer2PeerResult
	pCRedisHandle      *DB.CRedisHandle
}

// 创建新的Peer2PeerManager
func NewPeer2PeerManager() *Peer2PeerManager {
	pPeer2PeerManager := &Peer2PeerManager{
		NatInfoMap:         make(map[string]NatInfo),
		Peer2PeerResultMap: make(map[string]Peer2PeerResult),
		pCRedisHandle:      DB.NewRedisHandle(),
	}
	err := pPeer2PeerManager.pCRedisHandle.Connect()
	if err != nil {
		FmtLog.ERR("pPeer2PeerManager.pCRedisHandle.Connect err %s,%d\r\n", pPeer2PeerManager.pCRedisHandle.Addr, pPeer2PeerManager.pCRedisHandle.Port)
	}
	return pPeer2PeerManager
}

// 处理消息
func (m *Peer2PeerManager) Proc(mgrQueue *ThreadMsgQueue) int {

	m.pMgrQueue = mgrQueue

	var msg QueueMessage
	if mgrQueue.WaitAndPop(&msg, 10) != 0 { // 10 ms超时
		return -1
	}

	switch msg.MsgID {
	case REPORT_NAT_INFO_MSG_ID:
		return m.handleReportNatInfo(msg)
	case GET_NAT_INFO_MSG_ID:
		return m.handleGetNatInfo(msg)
	case REQ_PEER_SEND_MSG_MSG_ID:
		return m.handleReqPeerSendMsg(msg)
	case REQ_SEND_MSG_TO_PEER_ACK_MSG_ID:
		return m.handleReqSendMsgToPeerAck(msg)
	case REPORT_P2P_RESULT_MSG_ID:
		return m.handleReportP2PResult(msg)
	default:
		FmtLog.ERR("Peer2PeerManager :: Proc msg.iMsgID err %d %d \n", msg.MsgID, msg.DataSize)
		return -1
	}
}

// 处理报告NAT信息
func (m *Peer2PeerManager) handleReportNatInfo(msg QueueMessage) int {
	var natInfoMsg NatInfoMsg
	if err := json.Unmarshal(msg.Data, &natInfoMsg); err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REPORT_NAT_INFO_MSG_ID unmarshal err %v\n", err)
		return -1
	}

	natInfo := NatInfo{
		pSession:    msg.Sender,
		iNatType:    natInfoMsg.NatType,
		iPublicPort: natInfoMsg.PublicPort,
		strPublicIP: natInfoMsg.PublicIP,
	}

	if existing, ok := m.NatInfoMap[natInfoMsg.ID]; ok {
		existing.pSession = natInfo.pSession
		existing.iNatType = natInfo.iNatType
		existing.iPublicPort = natInfo.iPublicPort
		existing.strPublicIP = natInfo.strPublicIP
		m.NatInfoMap[natInfoMsg.ID] = existing
	} else {
		m.NatInfoMap[natInfoMsg.ID] = natInfo
	}
	// 要存入的map（string类型的键值对）
	data := map[string]interface{}{
		"PublicIP":   m.NatInfoMap[natInfoMsg.ID].strPublicIP,
		"PublicPort": m.NatInfoMap[natInfoMsg.ID].iPublicPort,
		"NatType":    m.NatInfoMap[natInfoMsg.ID].iNatType,
	}
	m.pCRedisHandle.Write("P2PDev:"+natInfoMsg.ID, data, -1)
	return 0
}

// 处理获取NAT信息
func (m *Peer2PeerManager) handleGetNatInfo(msg QueueMessage) int {
	peerID := string(msg.Data)
	FmtLog.DBG("Peer2PeerManager :: Proc GET_NAT_INFO_MSG_ID peerID %s \n", peerID)

	natInfo, ok := m.NatInfoMap[peerID]

	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc m_NatInfoMap find err %s \n", peerID)
		return -1
	}

	natInfoMsg := NatInfoMsg{
		ID:         peerID,
		NatType:    natInfo.iNatType,
		PublicPort: natInfo.iPublicPort,
		PublicIP:   natInfo.strPublicIP,
	}

	data, err := json.Marshal(natInfoMsg)
	if err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc GET_NAT_INFO_ACK_MSG_ID marshal err %v\n", err)
		return -1
	}

	queue, ok := msg.Sender.(*ThreadMsgQueue)
	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc invalid queue type")
		return -1
	}

	ackMsg := QueueMessage{
		Data:     data,
		DataSize: len(data),
		MsgID:    GET_NAT_INFO_ACK_MSG_ID,
	}

	if ret := queue.Push(ackMsg); ret < 0 {
		FmtLog.ERR("Peer2PeerManager::Proc GET_NAT_INFO_ACK_MSG_ID err %d \n", ret)
		return ret
	}

	return 0
}

// 处理请求对等发送消息
func (m *Peer2PeerManager) handleReqPeerSendMsg(msg QueueMessage) int {
	var reqPeerSendMsg ReqPeerSendMsg
	if err := json.Unmarshal(msg.Data, &reqPeerSendMsg); err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REQ_PEER_SEND_MSG_MSG_ID unmarshal err %v\n", err)
		return -1
	}

	natInfo, ok := m.NatInfoMap[reqPeerSendMsg.PeerID]

	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc m_NatInfoMap find strPeerID err %s \n", reqPeerSendMsg.PeerID)
		return -1
	}

	queue, ok := natInfo.pSession.(*ThreadMsgQueue)
	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc invalid queue type")
		return -1
	}

	data, err := json.Marshal(reqPeerSendMsg.LocalNatInfo)
	if err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REQ_SEND_MSG_TO_PEER_MSG_ID marshal err %v\n", err)
		return -1
	}

	sendMsg := QueueMessage{
		Data:     data,
		DataSize: len(data),
		MsgID:    REQ_SEND_MSG_TO_PEER_MSG_ID,
		Sender:   msg.Sender,
	}

	if ret := queue.Push(sendMsg); ret < 0 {
		FmtLog.ERR("Peer2PeerManager::Proc REQ_SEND_MSG_TO_PEER_MSG_ID err %d \n", ret)
		return ret
	}

	return 0
}

// 处理请求发送消息到对等确认
func (m *Peer2PeerManager) handleReqSendMsgToPeerAck(msg QueueMessage) int {
	var resultMsg ReqSendMsgToPeerResultMsg
	if err := json.Unmarshal(msg.Data, &resultMsg); err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REQ_SEND_MSG_TO_PEER_ACK_MSG_ID unmarshal err %v\n", err)
		return -1
	}

	peerNatInfo, ok1 := m.NatInfoMap[resultMsg.PeerID]
	localNatInfo, ok2 := m.NatInfoMap[resultMsg.LocalID]

	if !ok1 {
		FmtLog.ERR("Peer2PeerManager :: Proc m_NatInfoMap find strPeerID err %s \n", resultMsg.PeerID)
		return -1
	}
	if !ok2 {
		FmtLog.ERR("Peer2PeerManager :: Proc m_NatInfoMap find strLocalID err %s \n", resultMsg.LocalID)
		return -1
	}

	ackMsg := ReqPeerSendAckMsg{
		Result: resultMsg.Result,
		PeerNatInfo: NatInfoMsg{
			ID:         resultMsg.LocalID,
			NatType:    localNatInfo.iNatType,
			PublicPort: localNatInfo.iPublicPort,
			PublicIP:   localNatInfo.strPublicIP,
		},
	}

	data, err := json.Marshal(ackMsg)
	if err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REQ_PEER_SEND_MSG_ACK_MSG_ID marshal err %v\n", err)
		return -1
	}

	queue, ok := peerNatInfo.pSession.(*ThreadMsgQueue)
	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc invalid queue type")
		return -1
	}

	sendAckMsg := QueueMessage{
		Data:     data,
		DataSize: len(data),
		MsgID:    REQ_PEER_SEND_MSG_ACK_MSG_ID,
	}

	if ret := queue.Push(sendAckMsg); ret < 0 {
		FmtLog.ERR("Peer2PeerManager::Proc REQ_PEER_SEND_MSG_ACK_MSG_ID err %d \n", ret)
		return ret
	}

	return 0
}

// 处理报告P2P结果
func (m *Peer2PeerManager) handleReportP2PResult(msg QueueMessage) int {
	var resultMsg ReqSendMsgToPeerResultMsg
	if err := json.Unmarshal(msg.Data, &resultMsg); err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REPORT_P2P_RESULT_MSG_ID unmarshal err %v\n", err)
		return -1
	}

	key := resultMsg.LocalID + resultMsg.PeerID

	var successCnt, failCnt, curStatus int
	if result, ok := m.Peer2PeerResultMap[key]; ok {
		if resultMsg.Result < 0 {
			result.iFailCnt++
			result.iCurStatus = -1
		} else {
			result.iSuccessCnt++
			result.iCurStatus = 0
		}
		successCnt = result.iSuccessCnt
		failCnt = result.iFailCnt
		curStatus = result.iCurStatus
		m.Peer2PeerResultMap[key] = result
	} else {
		result := Peer2PeerResult{
			strLocalID: resultMsg.LocalID,
			strPeerID:  resultMsg.PeerID,
		}
		if resultMsg.Result < 0 {
			result.iFailCnt = 1
			result.iCurStatus = -1
		} else {
			result.iSuccessCnt = 1
			result.iCurStatus = 0
		}
		successCnt = result.iSuccessCnt
		failCnt = result.iFailCnt
		curStatus = result.iCurStatus
		m.Peer2PeerResultMap[key] = result
	}

	p2pResultMsg := PeerToPeerResultMsg{
		LocalID:    resultMsg.LocalID,
		PeerID:     resultMsg.PeerID,
		SuccessCnt: successCnt,
		FailCnt:    failCnt,
		CurStatus:  curStatus,
	}

	data, err := json.Marshal(p2pResultMsg)
	if err != nil {
		FmtLog.ERR("Peer2PeerManager :: Proc REPORT_P2P_RESULT_ACK_MSG_ID marshal err %v\n", err)
		return -1
	}

	queue, ok := msg.Sender.(*ThreadMsgQueue)
	if !ok {
		FmtLog.ERR("Peer2PeerManager :: Proc invalid queue type")
		return -1
	}

	resultAckMsg := QueueMessage{
		Data:     data,
		DataSize: len(data),
		MsgID:    REPORT_P2P_RESULT_ACK_MSG_ID,
	}

	if ret := queue.Push(resultAckMsg); ret < 0 {
		FmtLog.ERR("Peer2PeerManager::Proc REPORT_P2P_RESULT_ACK_MSG_ID err %d \n", ret)
		return ret
	}
	// 要存入的map（string类型的键值对）
	dataR := map[string]interface{}{
		"LocalID":    m.Peer2PeerResultMap[key].strLocalID,
		"PeerID":     m.Peer2PeerResultMap[key].strPeerID,
		"SuccessCnt": m.Peer2PeerResultMap[key].iSuccessCnt,
		"FailCnt":    m.Peer2PeerResultMap[key].iFailCnt,
		"CurStatus":  m.Peer2PeerResultMap[key].iCurStatus,
	}
	m.pCRedisHandle.Write("P2PResult:"+key, dataR, -1)
	return 0
}

// GetNatInfo 获取NAT信息
func (m *Peer2PeerManager) GetNatInfo(id string) (NatInfo, bool) {
	natInfo, ok := m.NatInfoMap[id]
	return natInfo, ok
}

// GetPeer2PeerResult 获取P2P结果
func (m *Peer2PeerManager) GetPeer2PeerResult(localID, peerID string) (Peer2PeerResult, bool) {
	key := localID + peerID
	result, ok := m.Peer2PeerResultMap[key]
	return result, ok
}

// ClearNatInfo 清除NAT信息
func (m *Peer2PeerManager) ClearNatInfo(id string) {
	delete(m.NatInfoMap, id)
}

// ClearPeer2PeerResult 清除P2P结果
func (m *Peer2PeerManager) ClearPeer2PeerResult(localID, peerID string) {

	key := localID + peerID
	delete(m.Peer2PeerResultMap, key)
}

// GetPeerNatInfo 获取对等NAT信息
func (m *Peer2PeerManager) GetPeerNatInfo(peerID string) (int, string, int, bool) {

	natInfo, ok := m.NatInfoMap[peerID]
	if !ok {
		return 0, "", 0, false
	}

	return natInfo.iNatType, natInfo.strPublicIP, natInfo.iPublicPort, true
}
