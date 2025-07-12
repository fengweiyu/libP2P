package libP2PServer //包名不需要和目录名相同，但在同一目录的文件必须声明同一包名

//在Go中，同一目录下的所有.go文件必须声明相同的包名（package关键字后的名字）。
//具体原因：Go代码编译时会将同一目录的文件视作一个包（package），这些文件必须声明相同的包名，才能在编译时组合成一个整体。
//如果不同文件声明不同的包名，编译器会报错，提示包名不一致。

//包内所有文件属于同一包，因此它们在逻辑上是“共享”的，可以直接调用彼此的未导出和导出的标识符（函数、方法、类型、字段）。
//导出（exported）：首字母大写的标识符可以在包外部被调用；首字母小写的私有（unexported）标识符只能在包内部使用。

import (
	"FmtLog" //库名和包名一样的情况，直接引用//如果一个库里有多个包，则要库/包的形式引用 ,一个项目（模块）通常由多个包组成//引用其他包必须前面带库(模块)名，如果库名和包名一样的情况，可以直接引用
	"encoding/json"
)

// P2P_LOGE 日志错误
func P2P_LOGE(format string, args ...interface{}) {
	FmtLog.ERR(format, args...)
}

// P2P_LOGD 日志调试
func P2P_LOGD(format string, args ...interface{}) {
	FmtLog.DBG(format, args...)
}

// P2P_LOGW 日志警告
func P2P_LOGW(format string, args ...interface{}) {
	FmtLog.WARN(format, args...)
}

type T_LoginReqMsgData struct {
	strLocalID string `json:"LocalID"`
}

type T_LoginReqMsg struct {
	strReq           string            `json:"req"`
	tLoginReqMsgData T_LoginReqMsgData `json:"data"`
}

// ServerSession 服务器会话结构体
type ServerSession struct {
	mgrQueue     *ThreadMsgQueue
	sessionQueue *ThreadMsgQueue
	localNatInfo NatInfoMsg
	localID      string
	peer2PeerCfg Peer2PeerCfg
}

// NewServerSession 创建新的ServerSession
func NewServerSession(mgrQueue *ThreadMsgQueue, cfg *Peer2PeerCfg) *ServerSession {
	session := &ServerSession{
		mgrQueue:     mgrQueue,
		sessionQueue: NewThreadMsgQueue(), // 假设有一个Go实现的ThreadSafeQueue
		peer2PeerCfg: *cfg,
	}
	return session
}
func (s *ServerSession) Proc(reqStr string, reqLen int, resBuffer []byte, resMaxLen int) int {
	ret := -1
	reqOrRes := -1
	cmdBuf := ""

	if resBuffer == nil {
		P2P_LOGE("ServerSession::Proc NULL err %d \r\n", resMaxLen)
		return ret
	}

	if reqStr != "" && reqLen > 0 {
		reqOrRes, cmdBuf, ret = s.ParseClientMsg(reqStr)
		if ret < 0 {
			P2P_LOGE("ServerSession::Proc ParseServerMsg err %d,%s\r\n", reqLen, reqStr)
			return ret
		}
		P2P_LOGD("ParseClientMsg LocalID %s,%s \r\n", s.localID, reqStr)
	}

	if reqOrRes == 1 { // res
		if cmdBuf == "SendMsgToPeer" {
			result := ReqSendMsgToPeerResultMsg{}
			localID, peerID, ret := s.ParseSendMsgToPeerRes(reqStr)
			result.LocalID = localID
			result.PeerID = peerID
			if ret < 0 || result.LocalID != s.localID {
				P2P_LOGE("ServerSession::Proc ParseSendMsgToPeerRes err %d ,%s ,%s\r\n", ret, result.LocalID, s.localID)
				ret = -1
			}
			result.Result = ret

			data, _ := json.Marshal(result)
			msg := QueueMessage{
				MsgID:    REQ_SEND_MSG_TO_PEER_ACK_MSG_ID,
				Data:     data,
				DataSize: len(data),
				Sender:   s.sessionQueue,
			}
			ret = s.mgrQueue.Push(msg)
			return ret
		}
		return ret
	}

	if reqOrRes == 0 && cmdBuf == "login" {
		localID, ret := s.ParseLoginReq(reqStr)
		s.localID = localID
		if ret < 0 {
			P2P_LOGE("ServerSession::Proc ParseLoginReq err %d \r\n", ret)
			return ret
		}
		resStr := s.CreateLoginRes(&s.peer2PeerCfg)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreateLoginRes err \r\n")
			return -1
		}
		copy(resBuffer, []byte(resStr))
		return len(resStr)
	}

	if reqOrRes == 0 && cmdBuf == "ReportNatInfo" {
		id, natType, publicIP, publicPort, ret := s.ParseReportNatInfoReq(reqStr)
		if id != s.localID || ret < 0 {
			P2P_LOGE("ServerSession::Proc ParseReportNatInfoReq err %d ,%s ,%s\r\n", ret, id, s.localID)
			return ret
		}

		s.localNatInfo = NatInfoMsg{
			ID:         id,
			NatType:    natType,
			PublicIP:   publicIP,
			PublicPort: publicPort,
		}

		data, _ := json.Marshal(s.localNatInfo)
		msg := QueueMessage{
			MsgID:    REPORT_NAT_INFO_MSG_ID,
			Data:     data,
			DataSize: len(data),
			Sender:   s.sessionQueue,
		}
		ret = s.mgrQueue.Push(msg)

		resStr := s.CreateReportNatInfoRes(ret)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreateReportNatInfoRes err \r\n")
			return -1
		}

		copy(resBuffer, []byte(resStr))
		return len(resStr)
	}

	if reqOrRes == 0 && cmdBuf == "PeerNatInfo" {
		peerID, ret := s.ParsePeerNatInfoReq(reqStr)
		if peerID == "" || ret < 0 {
			P2P_LOGE("ServerSession::Proc ParsePeerNatInfoReq err %d ,%s ,%s\r\n", ret, peerID, s.localID)
			return ret
		}

		msg := QueueMessage{
			MsgID:    GET_NAT_INFO_MSG_ID,
			Data:     []byte(peerID),
			DataSize: len(peerID),
			Sender:   s.sessionQueue,
		}
		ret = s.mgrQueue.Push(msg)
		if ret < 0 {
			P2P_LOGE("ServerSession::Proc GET_NAT_INFO_MSG_ID err %d \r\n", ret)
		}
		return ret
	}

	if reqOrRes == 0 && cmdBuf == "PeerSendMsg" {
		peerID, ret := s.ParsePeerSendMsgReq(reqStr)
		if ret < 0 {
			P2P_LOGE("ServerSession::Proc ParsePeerSendMsgReq err %d ,%s ,%s\r\n", ret, peerID, s.localID)
			return ret
		}

		reqPeerSendMsg := ReqPeerSendMsg{
			PeerID:       peerID,
			LocalNatInfo: s.localNatInfo,
		}

		data, _ := json.Marshal(reqPeerSendMsg)
		msg := QueueMessage{
			MsgID:    REQ_PEER_SEND_MSG_MSG_ID,
			Data:     data,
			DataSize: len(data),
			Sender:   s.sessionQueue,
		}
		ret = s.mgrQueue.Push(msg)
		return ret
	}

	if reqOrRes == 0 && cmdBuf == "ReportResult" {
		localID, peerID, ret := s.ParseReportResultReq(reqStr)
		if ret < 0 {
			P2P_LOGE("ServerSession::Proc ParseReportResultReq err %d ,strLocalID %s ,strPeerID %s\r\n", ret, localID, peerID)
		}

		result := ReqSendMsgToPeerResultMsg{
			LocalID: localID,
			PeerID:  peerID,
			Result:  ret,
		}

		data, _ := json.Marshal(result)
		msg := QueueMessage{
			MsgID:    REPORT_P2P_RESULT_MSG_ID,
			Data:     data,
			DataSize: len(data),
			Sender:   s.sessionQueue,
		}
		ret = s.mgrQueue.Push(msg)
		return ret
	}

	var msg QueueMessage
	if s.sessionQueue.WaitAndPop(&msg, 10) != 0 { // 10ms超时
		return ret
	}

	// 处理消息
	switch msg.MsgID {
	case GET_NAT_INFO_ACK_MSG_ID:
		natInfo := NatInfoMsg{}
		err := json.Unmarshal(msg.Data, &natInfo)
		if err != nil {
			P2P_LOGE("ServerSession :: Proc GET_NAT_INFO_ACK_MSG_ID unmarshal err %v\r\n", err)
			return ret
		}

		resStr := s.CreatePeerNatInfoRes(natInfo.ID, natInfo.NatType, natInfo.PublicIP, natInfo.PublicPort)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreatePeerNatInfoRes err \r\n")
			return -1
		}

		copy(resBuffer, []byte(resStr))
		ret = len(resStr)

	case REQ_SEND_MSG_TO_PEER_MSG_ID:
		peerNatInfo := NatInfoMsg{}
		err := json.Unmarshal(msg.Data, &peerNatInfo)
		if err != nil {
			P2P_LOGE("ServerSession :: Proc REQ_SEND_MSG_TO_PEER_MSG_ID unmarshal err %v\r\n", err)
			return ret
		}

		resStr := s.CreateSendMsgToPeerReq(peerNatInfo.ID, peerNatInfo.NatType, peerNatInfo.PublicIP, peerNatInfo.PublicPort)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreateSendMsgToPeerReq err \r\n")
			return -1
		}

		copy(resBuffer, []byte(resStr))
		ret = len(resStr)

	case REQ_PEER_SEND_MSG_ACK_MSG_ID:
		ackMsg := ReqPeerSendAckMsg{}
		err := json.Unmarshal(msg.Data, &ackMsg)
		if err != nil {
			P2P_LOGE("ServerSession :: Proc REQ_PEER_SEND_MSG_ACK_MSG_ID unmarshal err %v\r\n", err)
			return ret
		}

		resStr := s.CreatePeerSendMsgRes(ackMsg.Result, ackMsg.PeerNatInfo.ID, ackMsg.PeerNatInfo.NatType,
			ackMsg.PeerNatInfo.PublicIP, ackMsg.PeerNatInfo.PublicPort)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreatePeerSendMsgRes err \r\n")
			return -1
		}

		copy(resBuffer, []byte(resStr))
		ret = len(resStr)

	case REPORT_P2P_RESULT_ACK_MSG_ID:
		resultMsg := PeerToPeerResultMsg{}
		err := json.Unmarshal(msg.Data, &resultMsg)
		if err != nil {
			P2P_LOGE("ServerSession :: Proc REPORT_P2P_RESULT_ACK_MSG_ID unmarshal err %v\r\n", err)
			return ret
		}

		resStr := s.CreateReportResultRes(resultMsg.LocalID, resultMsg.PeerID, resultMsg.SuccessCnt,
			resultMsg.FailCnt, resultMsg.CurStatus)
		if resStr == "" {
			P2P_LOGE("ServerSession::Proc CreateReportResultRes err \r\n")
			return -1
		}

		copy(resBuffer, []byte(resStr))
		ret = len(resStr)

	default:
		P2P_LOGW("ServerSession::Proc iMsgID err %d \r\n", msg.MsgID)
	}

	if ret < 0 {
		P2P_LOGE("ServerSession::Proc err %d \r\n", ret)
	}
	return ret
}

// ParseClientMsg 解析客户端消息
func (s *ServerSession) ParseClientMsg(msgStr string) (reqOrRes int, cmdBuf string, ret int) {
	ret = -1
	reqOrRes = -1

	if msgStr == "" {
		P2P_LOGE("ParseClientMsg err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParseClientMsg json unmarshal err: %v\r\n", err)
		return
	}

	if res, ok := jsonData["res"].(string); ok {
		cmdBuf = res
		reqOrRes = 1
		ret = len(cmdBuf)
	} else if req, ok := jsonData["req"].(string); ok {
		cmdBuf = req
		reqOrRes = 0
		ret = len(cmdBuf)
	}

	return
}

// ParseLoginReq 解析登录请求
func (s *ServerSession) ParseLoginReq(msgStr string) (localID string, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParseLoginReq err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParseLoginReq json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParseLoginReq no data field\r\n")
		return
	}

	if id, ok := data["LocalID"].(string); ok {
		localID = id
		ret = len(localID)
	}

	return
}

// ParseReportNatInfoReq 解析报告NAT信息请求
func (s *ServerSession) ParseReportNatInfoReq(msgStr string) (id string, natType int, publicIP string, publicPort int, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParseReportNatInfoReq err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParseReportNatInfoReq json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParseReportNatInfoReq no data field\r\n")
		return
	}
	if localID, ok := data["LocalID"].(string); ok {
		id = localID
	}
	natInfo, ok := data["LocalNatInfo"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParseReportNatInfoReq no LocalNatInfo field\r\n")
		return
	}

	if nt, ok := natInfo["NatType"].(float64); ok {
		natType = int(nt)
	}
	if ip, ok := natInfo["PublicIP"].(string); ok {
		publicIP = ip
	}
	if port, ok := natInfo["PublicPort"].(float64); ok {
		publicPort = int(port)
	}

	ret = 0
	return
}

// ParsePeerNatInfoReq 解析对等NAT信息请求
func (s *ServerSession) ParsePeerNatInfoReq(msgStr string) (peerID string, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParsePeerNatInfoReq err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParsePeerNatInfoReq json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParsePeerNatInfoReq no data field\r\n")
		return
	}

	if id, ok := data["PeerID"].(string); ok {
		peerID = id
		ret = len(peerID)
	}

	return
}

// ParsePeerSendMsgReq 解析对等发送消息请求
func (s *ServerSession) ParsePeerSendMsgReq(msgStr string) (peerID string, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParsePeerSendMsgReq err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParsePeerSendMsgReq json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParsePeerSendMsgReq no data field\r\n")
		return
	}

	if id, ok := data["PeerID"].(string); ok {
		peerID = id
		ret = len(peerID)
	}

	return
}

// ParseSendMsgToPeerRes 解析发送消息到对等响应
func (s *ServerSession) ParseSendMsgToPeerRes(msgStr string) (localID string, peerID string, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParseSendMsgToPeerRes err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParseSendMsgToPeerRes json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParseSendMsgToPeerRes no data field\r\n")
		return
	}

	if id, ok := data["LocalID"].(string); ok {
		localID = id
	}

	if id, ok := data["PeerID"].(string); ok {
		peerID = id
	}

	if resultCode, ok := data["ResultCode"].(float64); ok {
		ret = int(resultCode)
	}

	return
}

// ParseReportResultReq 解析报告结果请求
func (s *ServerSession) ParseReportResultReq(msgStr string) (localID string, peerID string, ret int) {
	ret = -1

	if msgStr == "" {
		P2P_LOGE("ParseReportResultReq err: empty message\r\n")
		return
	}

	var jsonData map[string]interface{}
	err := json.Unmarshal([]byte(msgStr), &jsonData)
	if err != nil {
		P2P_LOGE("ParseReportResultReq json unmarshal err: %v\r\n", err)
		return
	}

	data, ok := jsonData["data"].(map[string]interface{})
	if !ok {
		P2P_LOGE("ParseReportResultReq no data field\r\n")
		return
	}

	if id, ok := data["LocalID"].(string); ok {
		localID = id
	}

	if id, ok := data["PeerID"].(string); ok {
		peerID = id
	}

	if resultCode, ok := data["ResultCode"].(float64); ok {
		ret = int(resultCode)
	}

	return
}

// CreateLoginRes 创建登录响应
func (s *ServerSession) CreateLoginRes(cfg *Peer2PeerCfg) string {
	if cfg == nil || cfg.StunServer1Addr == "" || cfg.StunServer2Addr == "" {
		P2P_LOGE("CreateLoginRes err: invalid config\r\n")
		return ""
	}

	response := map[string]interface{}{
		"res": "login",
		"data": map[string]interface{}{
			"StunServer1Addr": cfg.StunServer1Addr,
			"StunServer1Port": cfg.StunServer1Port,
			"StunServer2Addr": cfg.StunServer2Addr,
			"StunServer2Port": cfg.StunServer2Port,
		},
	}

	jsonData, err := json.Marshal(response)
	if err != nil {
		P2P_LOGE("CreateLoginRes json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}

// CreateReportNatInfoRes 创建报告NAT信息响应
func (s *ServerSession) CreateReportNatInfoRes(resultCode int) string {
	response := map[string]interface{}{
		"res": "ReportNatInfo",
		"data": map[string]interface{}{
			"ResultCode": resultCode,
			"ResultDesc": func() string {
				if resultCode == 0 {
					return "SUCCESS"
				}
				return "FAIL"
			}(),
		},
	}

	jsonData, err := json.Marshal(response)
	if err != nil {
		P2P_LOGE("CreateReportNatInfoRes json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}

// CreatePeerNatInfoRes 创建对等NAT信息响应
func (s *ServerSession) CreatePeerNatInfoRes(id string, natType int, publicIP string, publicPort int) string {
	if id == "" || publicIP == "" {
		P2P_LOGE("CreatePeerNatInfoRes err: invalid parameters\r\n")
		return ""
	}

	response := map[string]interface{}{
		"res": "PeerNatInfo",
		"data": map[string]interface{}{
			"PeerID": id,
			"PeerNatInfo": map[string]interface{}{
				"NatType":    natType,
				"PublicIP":   publicIP,
				"PublicPort": publicPort,
			},
		},
	}

	jsonData, err := json.Marshal(response)
	if err != nil {
		P2P_LOGE("CreatePeerNatInfoRes json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}

// CreatePeerSendMsgRes 创建对等发送消息响应
func (s *ServerSession) CreatePeerSendMsgRes(resultCode int, id string, natType int, publicIP string, publicPort int) string {
	if id == "" || publicIP == "" {
		P2P_LOGE("CreatePeerSendMsgRes err: invalid parameters\r\n")
		return ""
	}

	response := map[string]interface{}{
		"res": "PeerSendMsg",
		"data": map[string]interface{}{
			"PeerID": id,
			"PeerNatInfo": map[string]interface{}{
				"NatType":    natType,
				"PublicIP":   publicIP,
				"PublicPort": publicPort,
			},
			"ResultCode": resultCode,
			"ResultDesc": func() string {
				if resultCode == 0 {
					return "SUCCESS"
				}
				return "FAIL"
			}(),
		},
	}

	jsonData, err := json.Marshal(response)
	if err != nil {
		P2P_LOGE("CreatePeerSendMsgRes json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}

// CreateSendMsgToPeerReq 创建发送消息到对等请求
func (s *ServerSession) CreateSendMsgToPeerReq(peerID string, natType int, publicIP string, publicPort int) string {
	if peerID == "" || publicIP == "" {
		P2P_LOGE("CreateSendMsgToPeerReq err: invalid parameters\r\n")
		return ""
	}

	request := map[string]interface{}{
		"req": "SendMsgToPeer",
		"data": map[string]interface{}{
			"PeerID": peerID,
			"PeerNatInfo": map[string]interface{}{
				"NatType":    natType,
				"PublicIP":   publicIP,
				"PublicPort": publicPort,
			},
		},
	}

	jsonData, err := json.Marshal(request)
	if err != nil {
		P2P_LOGE("CreateSendMsgToPeerReq json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}

// CreateReportResultRes 创建报告结果响应
func (s *ServerSession) CreateReportResultRes(localID string, peerID string, successCnt int, failCnt int, curStatus int) string {
	if localID == "" || peerID == "" {
		P2P_LOGE("CreateReportResultRes err: invalid parameters\r\n")
		return ""
	}

	response := map[string]interface{}{
		"res": "ReportResult",
		"data": map[string]interface{}{
			"LocalID":    localID,
			"PeerID":     peerID,
			"SuccessCnt": successCnt,
			"FailCnt":    failCnt,
			"CurStatus": func() string {
				if curStatus == 0 {
					return "SUCCESS"
				}
				return "FAIL"
			}(),
		},
	}

	jsonData, err := json.Marshal(response)
	if err != nil {
		P2P_LOGE("CreateReportResultRes json marshal err: %v\r\n", err)
		return ""
	}

	return string(jsonData)
}
