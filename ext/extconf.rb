require 'mkmf'

dir_config("rbevent")
if have_library("event") and
    have_header("time.h")
    have_header("event.h")
    create_makefile("rbevent")
end

