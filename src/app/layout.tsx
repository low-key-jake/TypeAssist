import type { Metadata } from 'next';
import './globals.css';

export const metadata: Metadata = {
  title: 'TypeAssist — Think it. Type it. Transform it.',
  description: 'An intelligent writing assistant for every thought. Powered by C WebAssembly.',
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en" className="dark">
      <body className="min-h-screen antialiased">
        {children}
      </body>
    </html>
  );
}
