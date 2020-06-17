setTimeout(function() {
    sys.event.on('input', function(text) {
        if (text) {
            try {
                console.log(eval(atob(text)));
            } catch (e) {
                console.error(e);
            }
        }
        sys.config({
            'input': {
                'enable': true
            }
        });
    });
    console.log('Please input JavaScript code: ');
    sys.config({
        'input': {
            'enable': true
        }
    });
}, 4000);