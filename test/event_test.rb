require 'rbevent'
require 'time'
require 'test/unit'

include(RubyEvent)
include(RubyEvent::Constants)


class EventTest < Test::Unit::TestCase
    def setup
        event_init
    end
    
    def test_evtype
        timeout = 1
        ev = Event.new(-1, 0){|evt|
            assert_equal(evt.evtype, EV_TIMEOUT)
        }
        ev.add(timeout)
        event_dispatch
    end
    
    def test_fileno
        timeout = 1
        now = Time.now
        ev = Event.new(-1, 0){|evt|
            after = Time.now
            assert_equal(evt.fileno, -1)
            assert_equal(now.sec + timeout, after.sec)
        }
        ev.add(timeout)
        event_dispatch
    end

    def test_timeout1
        timeout = 5
        now = Time.now
        ev = Event.new(-1, 0){|evt|
            #p evt.fileno
            after = Time.now
            assert_equal(now.sec + timeout, after.sec)
        }
        ev.add(timeout)
        event_dispatch
    end

    def test_timeout2
        timeout = 5
        now = Time.now
        ev = TimerEvent.new(timeout){|evt|
            after = Time.now
            assert_equal(now.sec + timeout, after.sec)
        }
        event_dispatch
    end

    def test_signal1      
        cb = Proc.new{|evt|
            if evt.evtype == EV_SIGNAL
                evt.delete
            elsif evt.evtype == EV_TIMEOUT
                Process.kill(:USR1, Process.pid)
            end
        }
        SignalEvent.new(:USR1, EV_SIGNAL, &cb).add()
        Event.new(-1, 0, &cb).add(2)
        event_dispatch

    end

    def test_signal1      
        cb = Proc.new{|evt|
            if evt.evtype == EV_SIGNAL
                event_abort
            elsif evt.evtype == EV_TIMEOUT
                Process.kill(:USR1, Process.pid)
            end
        }
        SignalEvent.new(:USR1, EV_SIGNAL, &cb).add()
        TimerEvent.new(2, &cb)
        event_dispatch

    end
end
