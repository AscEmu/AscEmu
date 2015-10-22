/*
********************************************************************
Updates for accounts
********************************************************************
*/

-- Set all accounts to encrypted password
UPDATE accounts SET encrypted_password = SHA(CONCAT(UPPER(login),":",UPPER(password))) WHERE password <> '';

-- Delete cleartext column password
ALTER TABLE accounts DROP COLUMN password;
