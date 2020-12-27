
```
/*
 * 为什么不是直接一个全局的Application/other实例, 而是一个全部由虚函数
 * 实现的Context是因为为了能实现interface类型的解耦合, 通常Appliction/Other
 * 作为一个全局对象会存在很多与某些其他组建无关的逻辑和依赖, 很多时候我们
 * 在构建一个单元测试或者mock某部分的实现的时候, 我们不需要让整个Application
 * 都Ready, 可能仅仅只需要其中一个很小部分的功能, 所以通过继承AppContext 实现
 * 一个部分功能的Context对象便可以完成验证; 同时也为解耦合留下了可替换实现的
 * Interface,
 *
 * 考虑将实现自己Application的上线文环境,
 * 举个下面的例子:
 *
 * 在一个ServerApplication 的实现中, 我们总共由ComponentA, ComponentB, ComponentC三个组建
 * 一种解耦合的实现是各个组件抽象出自己所需的外部依赖InterfaceA, InterfaceB, InterfaceC
 *
 * ServerAppContext : AppContext {
 * private:
 *  InterfaceAImpliment a_impl_
 *  InterfaceBImpliment b_impl_
 *  InterfaceCImpliment c_impl_
 * }
 * 或者
 * ServerAppContext : AppContext, InterfaceA, InterfaceB, InterfaceC {
 * }
 *
 * 都可以做到各个组件建是解耦合的, 而整个Application又通过AppContext组合
 * 在一起. 在程序上下文进入ComponentA, ComponentB ComponentC 的过程时,
 * 带入的除了请求上下文外、组件的交互上线文(InterfaceX的Implement), 其他与
 * 组件无关的都可以隔离在组件外. 这对于程序不同部分之间保持清晰的逻辑界面非常
 * 有帮助.
 *
 * class ComponentX:
 *    function Run(ProcessRequestContext, WorkFlowNeededContext):
 *       // do something about ProcessRequestContext|WorkFlowNeededContext
 *
 * implement:
 * Class ApplicationHandler:
 *    function HandleRequest():
 *      context = BuildRequestContext;
 *
 *      componentA.Run(context, ServerAppContext.a_impl_):
 *
 *      componentB.Run(context, ServerAppContext.b_impl_):
 *
 *      componentC.Run(context, ServerAppContext.c_impl_):
 *      // or just wrapper the componen as list and do a foreach loop
 *
 * 背后的编码思想是自底向上, 而不是自顶向下开发. 在自顶向下的开发过程中
 * 总会是现有了Application实体. 最后需要做某个组件ComponentX实现业务逻辑
 * 而很容易显式或者隐式的将Application实体引入了ComponentX的依赖中, 而从
 * 设计上讲我们应该是
 * application:
 *  - Component : component 提供一定的交互接口来交互
 *    - 1. 正向依赖: 提供set_xxxx这样正向注入依赖或者其他方式, 这个思维逻辑所以
 *      不大有问题,也容易修改
 *    - 2. 反向依赖: 即Compoennt需要依赖外部某些实现,事情自己无法自己实现或者需要
 *      委托/依赖外部实现. 这一部分就需要我们实现依赖反转(Dependency inversion principle，DIP)
 *      简单的理解这是一种通过注入回调函数的是形式实现, 而OOP中,则是更加符合
 *      原则的Interface实现.
 *
 * 从而拒绝掉:
 * application、component 你中有我我中有你的耦合状态了.
 * */
```
