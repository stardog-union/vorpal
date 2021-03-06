#include "vowpalwabbit/vw.h"
#include "vowpalWabbit_learner_VWLearners.h"
#include "jni_base_learner.h"

#define RETURN_TYPE "vowpalWabbit/learner/VWLearners$VWReturnType"
#define RETURN_TYPE_INSTANCE "L" RETURN_TYPE ";"

JNIEXPORT jlong JNICALL Java_vowpalWabbit_learner_VWLearners_initialize(JNIEnv *env, jclass obj, jstring command)
{ jlong vwPtr = 0;
  const char* utf_string = env->GetStringUTFChars(command, NULL);
  try
  { vw* vwInstance = VW::initialize(utf_string);
    vwPtr = (jlong)vwInstance;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
  env->ReleaseStringUTFChars(command, utf_string);
  return vwPtr;
}

JNIEXPORT void JNICALL Java_vowpalWabbit_learner_VWLearners_closeInstance(JNIEnv *env, jclass obj, jlong vwPtr)
{ try
  { vw* vwInstance = (vw*)vwPtr;
    if (vwInstance->numpasses > 1)
      { adjust_used_index(*vwInstance);
        vwInstance->do_reset_source = true;
        VW::start_parser(*vwInstance);
        LEARNER::generic_driver(*vwInstance);
        VW::end_parser(*vwInstance);
      }
    VW::finish(*vwInstance);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jfloat JNICALL Java_vowpalWabbit_learner_VWLearners_exampleNumber(JNIEnv *env, jclass obj, jlong vwPtr)
{ try
  { vw* vwInstance = (vw*)vwPtr;
    return (vwInstance->sd->weighted_holdout_examples > 0) ? vwInstance->sd->weighted_holdout_examples : vwInstance->sd->weighted_examples;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jfloat JNICALL Java_vowpalWabbit_learner_VWLearners_sumLoss(JNIEnv *env, jclass obj, jlong vwPtr)
{ try
  { vw* vwInstance = (vw*)vwPtr;
    return (vwInstance->sd->weighted_holdout_examples > 0) ? vwInstance->sd->holdout_sum_loss : vwInstance->sd->sum_loss;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalWabbit_learner_VWLearners_saveModel(JNIEnv *env, jclass obj, jlong vwPtr, jstring filename)
{ try
  { const char* utf_string = env->GetStringUTFChars(filename, NULL);
    std::string filenameCpp(utf_string);
    env->ReleaseStringUTFChars(filename, utf_string);
    env->DeleteLocalRef(filename);
    VW::save_predictor(*(vw*)vwPtr, filenameCpp);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWLearners_getReturnType(JNIEnv *env, jclass obj, jlong vwPtr)
{ jclass clVWReturnType = env->FindClass(RETURN_TYPE);
  jfieldID field;
  vw* vwInstance = (vw*)vwPtr;
  switch (vwInstance->l->pred_type)
  { case prediction_type::prediction_type_t::action_probs:
      field = env->GetStaticFieldID(clVWReturnType , "ActionProbs", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::action_scores:
      field = env->GetStaticFieldID(clVWReturnType , "ActionScores", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::multiclass:
      field = env->GetStaticFieldID(clVWReturnType , "Multiclass", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::multilabels:
      field = env->GetStaticFieldID(clVWReturnType , "Multilabels", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::prob:
      field = env->GetStaticFieldID(clVWReturnType , "Prob", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::scalar:
      field = env->GetStaticFieldID(clVWReturnType , "Scalar", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::scalars:
      field = env->GetStaticFieldID(clVWReturnType , "Scalars", RETURN_TYPE_INSTANCE);
      break;
    default:
      field = env->GetStaticFieldID(clVWReturnType , "Unknown", RETURN_TYPE_INSTANCE);
  }

  return env->GetStaticObjectField(clVWReturnType, field);
}
