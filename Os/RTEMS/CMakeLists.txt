# Make sure Condition is registered separately, not as part of a list
register_os_implementation(File RTEMS)
register_os_implementation(FileSystem RTEMS)
register_os_implementation(Directory RTEMS)
register_os_implementation(Console RTEMS)
register_os_implementation(Task RTEMS)
register_os_implementation(Mutex RTEMS)
register_os_implementation(Queue RTEMS)
register_os_implementation(Cpu RTEMS)
register_os_implementation(Memory RTEMS)
register_os_implementation(RawTime RTEMS)
# register_os_implementation(Condition RTEMS)  # Register Condition separately
