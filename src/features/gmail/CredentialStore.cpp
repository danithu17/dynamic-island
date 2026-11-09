#include "CredentialStore.h"
#include <windows.h>
#include <wincred.h>
#include <QByteArray>

namespace Aura {

bool CredentialStore::storeSecret(const QString &targetName, const QString &secret) {
    std::wstring target = targetName.toStdWString();
    QByteArray utf8 = secret.toUtf8();

    CREDENTIALW cred = {0};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = const_cast<LPWSTR>(target.c_str());
    cred.CredentialBlobSize = static_cast<DWORD>(utf8.size());
    cred.CredentialBlob = reinterpret_cast<LPBYTE>(utf8.data());
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    cred.UserName = const_cast<LPWSTR>(L"AuraIslandUser");

    return CredWriteW(&cred, 0) == TRUE;
}

QString CredentialStore::readSecret(const QString &targetName) {
    std::wstring target = targetName.toStdWString();
    PCREDENTIALW pCred = nullptr;

    if (CredReadW(target.c_str(), CRED_TYPE_GENERIC, 0, &pCred) && pCred) {
        QString secret = QString::fromUtf8(reinterpret_cast<const char*>(pCred->CredentialBlob),
                                            pCred->CredentialBlobSize);
        CredFree(pCred);
        return secret;
    }
    return QString();
}

bool CredentialStore::deleteSecret(const QString &targetName) {
    std::wstring target = targetName.toStdWString();
    return CredDeleteW(target.c_str(), CRED_TYPE_GENERIC, 0) == TRUE;
}

} // namespace Aura
