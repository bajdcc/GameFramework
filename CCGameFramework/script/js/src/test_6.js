console.log(sys.http({
    url: "https://v1.hitokoto.cn/",
    callback: function(data) {
        var a = '你好，JSON！';
        console.log(a, a.length, a[0], a[1]);
        if (data.result === "success" && data.code === 200) {
            //console.log(JSON.stringify(data.headers, null, 2));
            //console.log(JSON.stringify(data.time, null, 2));
            console.log(JSON.stringify(JSON.parse(data.body), null, 2));
        }
    }
}));
console.log(sys.http({
    url: "http://music.163.com/song/media/outer/url?id=569962512.mp3",
    callback: function(data) {
        if (data.result === "success" && data.code === 200) {
            sys.music({
                "method": "play",
                "payload": {
                    "data": data.body
                }
            });
        }
    }
}));
setTimeout(function() {
    console.log(Buffer.from("123 你好", 'ascii'), Buffer.from(Buffer.from("123 你好")));
    console.log(Buffer.from([97, 98, 99, 100]));
}, 4000);