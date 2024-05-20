function fn() {
    try {
        throw "vaaa";
    } catch (e) {
        console.log('catch');
        while (1);
        return 123;
    } finally {
        console.log('finally')
        return 234;
    }
}
console.log(fn())