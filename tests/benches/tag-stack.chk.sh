#!/bin/sh

n=$1
it=$2
cat - > tag-stack.chk <<EOF

$(seq --format='var tag%0.0f = Tag.new |' 1 $n)
var tags = [$(seq --format='tag%0.0f' --separator=', ' 1 $n)] |

var reset = Tag.new | reset.freeze |
loop {
  reset:
  $(seq --format='tag%0.0f:' 1 $n) {
    // use an active wait otherwise this could hide the test result.
    for ($n) {
      1; 1; 1; 1; 1; 1; 1; 1;
      1; 1; 1; 1; 1; 1; 1; 1;
    }
  }
},

1;
[00000000] 1

for| ($it) {
  reset.unfreeze |

  tags[0].freeze |
  for| (var i: $n - 1) {
    tags[i + 1].freeze;
    1; 1; 1; 1; 1; 1; 1; 1;
    1; 1; 1; 1; 1; 1; 1; 1;
    tags[i].unfreeze;
  }|
  tags[$n - 1].unfreeze|

  reset.freeze | reset.stop |
}|

2;
[00000000] 2

for| ($it / 4) {
  reset.unfreeze |

  for| (var i: $n) {
    1; 1; 1; 1; 1; 1; 1; 1;
    1; 1; 1; 1; 1; 1; 1; 1;
    tags[i].stop;
  }|

  reset.freeze | reset.stop |
}|

"end";
[00000000] "end"

EOF
