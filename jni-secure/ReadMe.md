
获取Java方法签名
```jshelllanguage
javap -s MainActivity
```

比较字符串
```c++
int main() {
	char a[] = "aaa",b[]="aaa";
	string A = "AAA", B = "AAA";
	(a==b); // true
	strcmp(a,b); // 0
	(A==B); // false
	A.compare(B); // 0
	return 0;
}
```

`strcasecmp()`用来比较参数s1和s2字符串，比较时会自动忽略大小写的差异。
若参数s1和s2字符串相同则返回0。s1大于s2则返回大于0的值，s1小于s2则返回小于0的值。
`strncasecmp()`用来比较参数s1和s2字符串前n个字符，比较时会自动忽略大小写的差异。
若参数s1和s2字符串相同则返回0。s1大于s2则返回大于0的值，s1小于s2则返回小于0的值。

获取Application

Application和Activity是Context的子类，由于每个Activity对应的Context是不一样的，所以一般情况下我们使用Application的Context，它在整个程序中只有一个实例。那么怎么在JNI中获取Application呢？
Android APP在启动时会创建一个ActivityThread作为主线程，只要程序存活，这个线程就一直存在，所以我们可以考虑从ActivityThread中获取Application，查看ActivityThread的源码发现，它提供了一个方法可以获取Application，如下：

```java
public Application getApplication() {
    return mInitialApplication;
}
```
也就是说我们只需要获取到ActivityThread的对象即可，ActivityThread提供了一个静态方法用于获取其实例，如下：
```java
public static ActivityThread currentActivityThread() {
    return sCurrentActivityThread;
}
```

