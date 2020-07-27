function Promise(executor) {
    let self = this;
    self.status = 'pending';
    self.onFulfilled = [];
    self.onRejected = [];

    function resolve(value) {
        if (self.status === 'pending') {
            self.status = 'fulfilled';
            self.value = value;
            self.onFulfilled.forEach(fn => fn());
        }
    }

    function reject(reason) {
        if (self.status === 'pending') {
            self.status = 'rejected';
            self.reason = reason;
            self.onRejected.forEach(fn => fn());
        }
    }

    try {
        executor(resolve, reject);
    } catch (e) {
        reject(e);
    }
}

Promise.prototype.resolve = function(value) {
    if (value instanceof Promise) return value;
    return new Promise(resolve => resolve(value));
};

Promise.prototype.reject = function(value) {
    if (value instanceof Promise) return value;
    return new Promise((resolve, reject) => reject(value));
};

Promise.all = function(promises) {
    return new Promise((resolve, reject) => {
        nextPromise(0, promises);

        function nextPromise(index, promises) {
            let length = promises.length;
            if (index >= length) {
                resolve();
            }
            promises[index]()
                .then(() => {
                    nextPromise(index + 1, promises);
                }, err => {
                    reject(err);
                })
        }
    });
};

(function() {
    Promise.prototype.then = function(onFulfilled, onRejected) {
        onFulfilled = typeof onFulfilled === 'function' ? onFulfilled : value => value;
        onRejected = typeof onRejected === 'function' ? onRejected : reason => { throw reason };
        let self = this;
        let promise2 = new Promise((resolve, reject) => {
            if (self.status === 'fulfilled') {
                setTimeout(() => {
                    try {
                        let x = onFulfilled(self.value);
                        resolvePromise(promise2, x, resolve, reject);
                    } catch (e) {
                        reject(e);
                    }
                });
            } else if (self.status === 'rejected') {
                setTimeout(() => {
                    try {
                        let x = onRejected(self.reason);
                        resolvePromise(promise2, x, resolve, reject);
                    } catch (e) {
                        reject(e);
                    }
                });
            } else if (self.status === 'pending') {
                self.onFulfilled.push(() => {
                    setTimeout(() => {
                        try {
                            let x = onFulfilled(self.value);
                            resolvePromise(promise2, x, resolve, reject);
                        } catch (e) {
                            reject(e);
                        }
                    });
                });
                self.onRejected.push(() => {
                    setTimeout(() => {
                        try {
                            let x = onRejected(self.reason);
                            resolvePromise(promise2, x, resolve, reject);
                        } catch (e) {
                            reject(e);
                        }
                    });
                });
            }
        });
        return promise2;
    }

    function resolvePromise(promise2, x, resolve, reject) {
        let self = this;
        if (promise2 === x) {
            reject(new TypeError('Chaining cycle'));
        }
        if (x && typeof x === 'object' || typeof x === 'function') {
            let used;
            try {
                let then = x.then;
                if (typeof then === 'function') {
                    then.call(x, (y) => {
                        if (used) return;
                        used = true;
                        resolvePromise(promise2, y, resolve, reject);
                    }, (r) => {
                        if (used) return;
                        used = true;
                        reject(r);
                    });

                } else {
                    if (used) return;
                    used = true;
                    resolve(x);
                }
            } catch (e) {
                if (used) return;
                used = true;
                reject(e);
            }
        } else {
            resolve(x);
        }
    }
})();
return;