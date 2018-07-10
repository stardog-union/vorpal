Vorpal
====================
 
Java interface for [Vowpal Wabbit](https://github.com/JohnLangford/vowpal_wabbit/wiki).

Built upon the original [JNI code](https://github.com/JohnLangford/vowpal_wabbit/tree/master/java) from Vowpal Wabbit.


### Usage

```
VWScalarsLearner vw = VWLearners.create("--oaa 3 --holdout_period 9 --passes 10 --cache_file vw.cache --loss_function=logistic --probabilities");

vw.learn("1 | c d e");
float[] pred = vw.predict("c d e");

vw.saveModel("model.vw");
vw.close();
```

### TODO: Build instructions
