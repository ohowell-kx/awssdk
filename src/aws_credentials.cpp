// AWS headers first; the project headers below pull in k.h, which must come last.
#include <aws/core/auth/AWSCredentialsProviderChain.h>

#include "aws_session.h"
#include "aws_credentials.h"


namespace awssdk {
    K getCredentials(K) {
        if (!AwsSession::getInstance().isInitialized()) {
            return krr("uninitialized");
        }

        // provider is not cached -> will always fetch new credentials
        Aws::Auth::DefaultAWSCredentialsProviderChain provider;
        Aws::Auth::AWSCredentials creds = provider.GetAWSCredentials();

        std::string accessKey = creds.GetAWSAccessKeyId();
        std::string secretKey = creds.GetAWSSecretKey();
        std::string sessionToken = creds.GetSessionToken();
        std::string accountId = creds.GetAccountId();
        Aws::Utils::DateTime expiration = creds.GetExpiration();

        // ss/kp take S (char*) and copy their argument, so the casts are safe.
        K keys = ktn(KS, 5);
        kS(keys)[0] = ss((S)"accessKey");
        kS(keys)[1] = ss((S)"secretKey");
        kS(keys)[2] = ss((S)"sessionToken");
        kS(keys)[3] = ss((S)"accountId");
        kS(keys)[4] = ss((S)"expiration");

        K values = knk(5,
            kp((S)accessKey.c_str()),
            kp((S)secretKey.c_str()),
            kp((S)sessionToken.c_str()),
            kp((S)accountId.c_str()),
            kj(expiration.Millis()));

        return xD(keys, values);
   }
}
