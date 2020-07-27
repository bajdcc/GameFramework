(function() {
    function play(id) {
        var _data = {};
        const load_info = () => new Promise((resolve, reject) => {
            sys.http({
                method: 'post',
                url: "http://music.163.com/api/song/detail?id=" + id + "&ids=[" + id + "]",
                callback: function(data) {
                    if (data.result === "success" && data.code === 200) {
                        var obj = JSON.parse(data.body);
                        if (obj.code === 200 && obj.songs.length) {
                            _data.song = obj.songs[0];
                            return resolve();
                        }
                    }
                    reject('api info error');
                }
            })
        });
        const load_lyric = () => new Promise((resolve, reject) => {
            sys.http({
                method: 'get',
                url: "http://music.163.com/api/song/media?id=" + id,
                callback: function(data) {
                    if (data.result === "success" && data.code === 200) {
                        var obj = JSON.parse(data.body);
                        if (obj.code === 200) {
                            _data.lyric = obj.nolyric ? '' : obj.lyric;
                            return resolve();
                        }
                    } else {
                        reject('api lyric error');
                    }
                }
            })
        });
        const load_mp3 = () => new Promise((resolve, reject) => {
            sys.http({
                method: 'get',
                url: "http://music.163.com/song/media/outer/url?id=" + id + ".mp3",
                callback: function(data) {
                    if (data.result === "success" && data.code === 200) {
                        _data.music = data.body;
                        return resolve();
                    } else {
                        reject('api mp3 error');
                    }
                }
            })
        });
        const play_music = () => new Promise((resolve, reject) => {
            sys.music({
                "method": "play",
                "payload": {
                    "data": _data.music
                }
            });
            console.log('play:', _data.song.name);
            resolve();
        });
        const play_lyric = () => new Promise((resolve, reject) => {
            if (_data.lyric.length) {
                _data.ui = new UI({
                    type: 'label',
                    color: '#ffffff',
                    content: '',
                    left: 210,
                    top: 10,
                    width: 400,
                    height: 20,
                    align: 'left',
                    valign: 'center',
                    font: {
                        family: 'KaiTi',
                        size: 24
                    }
                });
                UI.root.push(_data.ui);
                var lyric = _data.lyric.split(/\n/g);
                _data.L = lyric.map(x => {
                    var str = x.match(/\[((\d+):(\d+)\.(\d+))\](.*)/);
                    var min = parseInt(str[2]);
                    var sec = parseInt(str[3]);
                    var msec = parseInt(str[4]) * 10;
                    var txt = str[5];
                    return {
                        time: min * 60000 + sec * 1000 + msec,
                        text: txt,
                        t: str[1]
                    };
                });
                _data.play_index = 0;
                _data.play_time = sys.get_config('time/timestamp');
                setTimeout(function auto_play() {
                    if (_data.play_index >= _data.L) return;
                    var diff = sys.get_config('time/timestamp') - _data.play_time;
                    var now = _data.L[_data.play_index];
                    if (diff >= now.time) {
                        console.log(diff, now.time, now.t, now.text);
                        _data.play_index++;
                        _data.ui.content = now.text;
                    }
                    setTimeout(auto_play, Math.max(now.time - diff, 10));
                }, 0);
            }
            resolve();
        });
        Promise.all([load_info, load_lyric, load_mp3, play_music, play_lyric]);
    }
    play('529823971');
})();