package FmtLog //包名类似命名空间或目录
//包名不需要和目录名相同，但在同一目录的文件必须声明同一包名

//在Go中，同一目录下的所有.go文件必须声明相同的包名（package关键字后的名字）。
//具体原因：Go代码编译时会将同一目录的文件视作一个包（package），这些文件必须声明相同的包名，才能在编译时组合成一个整体。
//如果不同文件声明不同的包名，编译器会报错，提示包名不一致。

//包内所有文件属于同一包，因此它们在逻辑上是“共享”的，可以直接调用彼此的未导出和导出的标识符（函数、方法、类型、字段）。
//导出（exported）：首字母大写的标识符可以在包外部被调用；首字母小写的私有（unexported）标识符只能在包内部使用。

import (
	"log"
)

func CRT(format string, a ...interface{}) {
	log.Printf("[CRIT]"+format, a...)
}

func ERR(format string, a ...interface{}) {
	log.Printf("[ERR]"+format, a...)
}

func DBG(format string, a ...interface{}) {
	log.Printf("[DBG]"+format, a...)
}

func INFO(format string, a ...interface{}) {
	log.Printf("[INFO]"+format, a...)
}

func WARN(format string, a ...interface{}) {
	log.Printf("[WARN]"+format, a...)
}
