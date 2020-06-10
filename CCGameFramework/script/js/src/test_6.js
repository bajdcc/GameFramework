console.log(sys.http({
    url: "https://v1.hitokoto.cn/",
    callback: function(data) {
        var a = '你好，JSON！';
        console.log(a, a.length, a[0], a[1]);
        if (data.result === "success" && data.code === 200)
            console.log(JSON.stringify(JSON.parse(data.body), null, 2));
    }
}));