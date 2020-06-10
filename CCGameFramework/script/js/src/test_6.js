console.log(sys.http({
    url: "https://v1.hitokoto.cn/",
    callback: function(data) {
        if (data.result === "success" && data.code === 200)
            console.log(JSON.stringify(JSON.parse(data.body), null, 2));
    }
}));