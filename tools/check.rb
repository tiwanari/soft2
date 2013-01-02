#! ruby -Ku
require 'net/http'
require 'uri'
url = URI.parse('http:://www.logos.t.u-tokyo.ac.jp/~itamochi/software2/kadai_20121220.html')
req = Net::HTTP::Get.new(url.path)
res = Net::HTTP.start(url.host, url.port) {|http|
    http.request(req)
}
print res.body
