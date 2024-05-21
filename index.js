fun_start("add")
    arg(0, 0) // val(0) = arg[0]
    arg(1, 1) // val(1) = arg[1]
    val(1,2,0) // val(2) = create(type(1),0)
    opt(OPT_ADD, 0, 1) // val(0)+val(1)
    set(2, -1) // val(2) = val(-1)#result
    ret(2) // return val(2)
fun_end()

val(1, 0, 1) // val(0) = create(type(1),1)
val(1, 1, 2) // val(1) = create(type(1),2)
val(1, 2, 0) // val(2) = create(type(1),0)
call("add", 0, 1) // add(0,1)
set(2, -1) //val(2) = val(-1)#result
call("print",2) // print(2)