#pragma once

#include <QString>

namespace Aura {

class CredentialStore {
public:
    static bool storeSecret(const QString &targetName, const QString &secret);
    static QString readSecret(const QString &targetName);
    static bool deleteSecret(const QString &targetName);
};

} // namespace Aura
