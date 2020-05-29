console.error((function () {
    try {
        throw new Error("error");
        return 1;
    } catch (e) {
        return e;
    }
    return 2;
})());
console.log((function () {
    try {
        throw 2;
    } catch (e) {
        return 1;
    } finally {
        return 2;
    }
    return 3;
})());
console.log((function () {
    try {
        throw new Error("error");
        return 1;
    } catch (e) {
        return 2;
    }
    return 3;
})());
console.log((function () {
    try {
        return 1;
    } finally {
        return 2;
    }
    return 3;
})());
console.log((function () {
    var s = 0;
    for (var i = 0; i < 2; i++) {
        try {
            s++;
            continue;
        } finally {
            s++;
            continue;
        }
        return 3;
    }
    return s;
})());
console.log((function () {
    var s = 0;
    switch (1) {
        case 1:
            try {
                s++;
                break;
            } finally {
                s++;
                break;
            }
            return 3;
    }
    return s;
})());
console.log((function () {
    try {
        try {
            throw new Error("error");
        } catch (e) {
            throw e;
        } finally {
            return 3;
        }
    } catch {
        return 4;
    } finally {
        return 5;
    }
})());
setTimeout(function () {
    try {
        undefined_error;
    } catch (e) {
        console.error(e);
    }
});