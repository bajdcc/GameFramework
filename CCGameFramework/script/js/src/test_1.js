console.log ((f => (g => n => f(g(g))(n))(g => n => f(g(g))(n)))
(f => n => n <= 1 ? 1 : n * f(n - 1))(5));