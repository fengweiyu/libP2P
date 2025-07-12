package DB

import (
	"context"
	"fmt"
	"time"

	"github.com/redis/go-redis/v9"
)

type CRedisHandle struct {
	Cli         *redis.Client
	Password    string
	Addr        string
	Port        int
	DialTimeout int
	Connected   bool
}

// NewThreadMsgQueue 创建一个新的线程安全队列
func NewRedisHandle() *CRedisHandle {
	return &CRedisHandle{
		Cli:         nil,
		Password:    "jftech987",
		Addr:        "122.9.73.57",
		Port:        5129,
		Connected:   false,
		DialTimeout: 5,
	}
}
func (r *CRedisHandle) Connect() error {
	if r.Connected {
		return nil
	}

	// 设置默认的连接超时时间
	if r.DialTimeout <= 0 {
		r.DialTimeout = 5
	}
	//ctx := context.Background() 的作用是创建一个没有截止时间和取消功能的空上下文，它是context包中的一种类型，提供了一个没有截止时间、没有附加值、没有取消的上下文环境。
	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(r.DialTimeout)*time.Second) //context.WithTimeout()：设置超时时间，超时后自动取消操作。
	defer cancel()

	s := fmt.Sprintf("%s:%d", r.Addr, r.Port)
	cli := redis.NewClient(&redis.Options{
		Addr:        s,
		Password:    r.Password,
		DialTimeout: time.Duration(r.DialTimeout) * time.Second,
		DB:          0,
	})
	err := cli.Ping(ctx).Err()
	if err != nil && err.Error() != "ERR Client sent AUTH, but no password is set" {
		r.Connected = false
		return err//会报超时错误暂时不用
	}
	r.Cli = cli
	r.Connected = true
	return nil
}
func (r *CRedisHandle) Read(strHashKey string) (mapVal string, ret int) {
	if !r.Connected {
		return "", -1
	}

	ctx := context.Background() //ctx := context.Background() 的作用是创建一个没有截止时间和取消功能的空上下文

	// 执行一个Redis命令，例如获取某个键
	val, err := r.Cli.Get(ctx, strHashKey).Result() //这个ctx在这里是作为参数传递的，是必须的。
	if err != nil && err != redis.Nil {
		// 连接或操作错误
		fmt.Println("Redis error:", err)
		return "", -1
	}

	return val, 0
}

// Pipeline()：只为性能提升、非原子性需求的批量操作//TxPipeline()：确保多条命令作为事务原子操作执行
func (r *CRedisHandle) Write(strHashKey string, mapVal map[string]interface{}, iTimeoutSecond int) (ret int) {
	if !r.Connected {
		return -1
	}

	ctx := context.Background()
	//Pipeline()不支持事务，即命令可以部分成功，部分失败，没有自动回滚机制//命令会尽快连续发送到Redis服务器 减少网络请求的延迟 不保证原子性
	// 启动事务（管道）  TxPipeline()在Redis中实现事务(transaction)：保证一组操作作为一个原子事务进行，要么全部成功，要么全部不执行
	pipe := r.Cli.TxPipeline() //通过MULTI/EXEC保证原子性 所有排队的命令会在EXEC时一起执行//使用TxPipeline()或TxPipeline()配合Exec()，完成事务操作。

	// 排队多个命令
	pipe.HSet(ctx, strHashKey, mapVal)
	//pipe.HSet(ctx, strHashKey1, mapVal1)//暂不支持多条后续可优化
	if iTimeoutSecond > 0 {
		pipe.Expire(ctx, strHashKey, time.Duration(iTimeoutSecond)*time.Second)
	}

	// 提交事务
	cmders, err := pipe.Exec(ctx)
	if err != nil {
		fmt.Println("事务执行失败:", err)
		return -1
	}
	// cmders包含所有命令的结果
	for _, cmd := range cmders {
		fmt.Println(cmd)
	}
	return 0
}

func (r *CRedisHandle) WriteD(strHashKey string, mapVal interface{}, iTimeoutSecond int) (ret int) {
	if !r.Connected {
		return -1
	}

	ctx := context.Background()

	// 执行一个Redis命令，
	err := r.Cli.HSet(ctx, strHashKey, mapVal).Err()
	if err != nil {
		fmt.Println("r.Cli.HSet err :", err)
		return -1
	}

	// 设置key过期时间为iTimeoutSecond秒
	if iTimeoutSecond > 0 {
		errR := r.Cli.Expire(ctx, strHashKey, time.Duration(iTimeoutSecond)*time.Second).Err()
		if errR != nil {
			fmt.Println("r.Cli.Expire err :", strHashKey)
			return -1
		}
	}
	return 0
}
