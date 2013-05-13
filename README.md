statsd-log-line-counter
=======================

deadly simple program that emits a hit to the statsd server for each line received.

It makes use of the [statsd-c-client](https://github.com/romanbsd/statsd-c-client) lib.


Options
-------

```
-N <metric> metric name to update (required)
-h <host>   statsd host (default: 127.0.0.1)
-p <post>   statsd port (default: 8125)
-s <rate>   sample rate (default: 1.00)
```

Usage example
-------------

sending errors to statsd

```
cat /var/log/httpd/error_log | ./log-lines-counter -N ops.apache.errors -h 172.16.16.134
```


sending apache hits to statsd at `0.20` sample rate so its **not too** intensive

```
cat /var/log/httpd/access_log | ./log-lines-counter -N ops.apache.errors -s 0.20
```


### Usage with syslog-ng ###

- Lets make it log!

```
log {   source(s_tcp);
        filter(f_apache_access);
        destination(d_statsd_apache_accesses); };
```

- Then setup the destination so `stastd` gets the info

```
destination d_statsd_apache_accesses {
    program("/usr/local/bin/log-lines-counter/log-lines-counter -N ops.apache.accesses -s 0.50" template("$MSG\n") );
};
```


License
-------

Copyright (c) 2013 Gabriel Sosa

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
